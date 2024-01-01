// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterComponents/CombatComponent.h"
#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/BlasterHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Weapon/Weapon.h"

#define TARGETING_RANGE 80000.f

UCombatComponent::UCombatComponent()
{
    // Enable tick so we can see the temporary debug trace
    PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    MirrorWalkSpeedBasedOnState();
}

void UCombatComponent::MirrorWalkSpeedBasedOnState() const
{
    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;
    }
}

void UCombatComponent::SetAiming(bool bInAiming)
{
    // bAiming is set here because if this is called on client then we will locally set var before calling server
    // not needed on server as ServerSetAiming falls directly through to the server implementation
    bAiming = bInAiming;
    // Set the Aiming flag on the server
    ServerSetAiming(bInAiming);
    MirrorWalkSpeedBasedOnState();
}

void UCombatComponent::ServerSetAiming_Implementation(bool bInAiming)
{
    bAiming = bInAiming;
    // Make sure we set the WalkSpeed set on the server and replicated to other clients
    MirrorWalkSpeedBasedOnState();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UCombatComponent::OnRep_EquippedWeapon()
{
    if (IsValid(EquippedWeapon) && IsValid(Character))
    {
        StopOrientingRotationToMovement();
    }
}

void UCombatComponent::SetFireButtonPressed(const bool bInFireButtonPressed)
{
    bFireButtonPressed = bInFireButtonPressed;
    if (bFireButtonPressed)
    {
        verify(Character->IsLocallyControlled());

        // Send fire action to the server
        // Note: It is unclear why we are not tracing on the server as this seems prone to cheating
        FHitResult HitResult;
        TraceUnderCrossHairs(HitResult);
        ServerFire(HitResult.ImpactPoint);
    }
}

void UCombatComponent::TraceUnderCrossHairs(FHitResult& OutHitResult) const
{
    if (GEngine && GEngine->GameViewport)
    {
        // We really only expect this to be invoked on locally controlled players
        ensure(Character && Character->IsLocallyControlled());

        FVector2D ViewportSize;
        GEngine->GameViewport->GetViewportSize(ViewportSize);

        // Crosshair is in the center of our viewport
        const FVector2D CrosshairViewportPosition(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

        // PlayerController 0 on ANY instance is the player character running the world (assuming multiple players
        // are NOT playing on one instance)
        auto const PlayerController = UGameplayStatics::GetPlayerController(this, 0);

        // Convert the Viewport space crosshair coordinate into a world space point
        FVector CrosshairWorldPosition;
        FVector CrosshairWorldDirection;
        if (UGameplayStatics::DeprojectScreenToWorld(PlayerController,
                                                     CrosshairViewportPosition,
                                                     CrosshairWorldPosition,
                                                     CrosshairWorldDirection))
        {
            const FVector Start{ CrosshairWorldPosition };
            // Create the end by starting at start and moving along the unit vector direction our TARGETING_RANGE
            const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * TARGETING_RANGE };

            GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_Visibility);

            if (!OutHitResult.bBlockingHit)
            {
                // If the trace does not hit anything then we set the OutHitResult.ImpactPoint to the end of our trace
                // As we know our caller is mostly just going to access this. Bad coding practice ... but such is life
                // when following tutorials
                OutHitResult.ImpactPoint = End;
            }
            // else
            // {
            //     // Draw a debug sphere at out hit location
            //     DrawDebugSphere(GetWorld(), OutHitResult.ImpactPoint, 12.f, 12, FColor::Red);
            // }
        }
    }
}

void UCombatComponent::SetHUDCrosshairs(const float DeltaTime)
{
    if (LIKELY(Character && Character->Controller))
    {
        if (UNLIKELY(!Controller))
        {
            Controller = Cast<ABlasterPlayerController>(Character->Controller);
        }
        if (UNLIKELY(!HUD))
        {
            HUD = Cast<ABlasterHUD>(Controller->GetHUD());
        }
        if (LIKELY(HUD))
        {
            FHUDPackage HUDPackage;
            if (EquippedWeapon)
            {
                HUDPackage.CrosshairsCenter = EquippedWeapon->GetCrosshairsCenter();
                HUDPackage.CrosshairsLeft = EquippedWeapon->GetCrosshairsLeft();
                HUDPackage.CrosshairsRight = EquippedWeapon->GetCrosshairsRight();
                HUDPackage.CrosshairsTop = EquippedWeapon->GetCrosshairsTop();
                HUDPackage.CrosshairsBottom = EquippedWeapon->GetCrosshairsBottom();

                // Map 0-> MaxWalkSpeed to 0->1 for current speed to get the spread factor
                const float CrosshairVelocityFactor =
                    FMath::GetMappedRangeValueClamped(FVector2D(0, Character->GetCharacterMovement()->MaxWalkSpeed),
                                                      FVector2D(0., 1.),
                                                      Character->GetVelocity().Size());

                if (Character->GetCharacterMovement()->IsFalling())
                {
                    // Target is larger than one as when we are falling there is a wider spread
                    constexpr float Target = 2.25f;
                    CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, Target, DeltaTime, 2.25f);
                }
                else
                {
                    // We slowly interp back to 0 when we land
                    constexpr float Target = 0.f;
                    CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, Target, DeltaTime, 30.f);
                }

                HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairInAirFactor;
            }
            else
            {
                HUDPackage.CrosshairsCenter = nullptr;
                HUDPackage.CrosshairsLeft = nullptr;
                HUDPackage.CrosshairsRight = nullptr;
                HUDPackage.CrosshairsTop = nullptr;
                HUDPackage.CrosshairsBottom = nullptr;
            }
            HUD->SetHUDPackage(HUDPackage);
        }
    }
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    // Send the fire action to all of the clients
    MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
    if (IsValid(Character) && IsValid(EquippedWeapon))
    {
        Character->PlayFireMontage(bAiming);
        EquippedWeapon->Fire(TraceHitTarget);
    }
}

void UCombatComponent::TickComponent(const float DeltaTime,
                                     const ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    SetHUDCrosshairs(DeltaTime);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCombatComponent, EquippedWeapon);
    DOREPLIFETIME(UCombatComponent, bAiming);
}

static const FName RightHandSocketName("RightHandSocket");

void UCombatComponent::StopOrientingRotationToMovement() const
{
    Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
    if (IsValid(Character) && IsValid(WeaponToEquip))
    {
        EquippedWeapon = WeaponToEquip;
        EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
        if (const auto HandSocket = Character->GetMesh()->GetSocketByName(RightHandSocketName))
        {
            HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
        }
        // Make sure we set owner of weapon so that weapon will be replicated wih player pawn
        EquippedWeapon->SetOwner(Character);
        EquippedWeapon->ShowPickupWidget(false);

        StopOrientingRotationToMovement();
    }
}
