#include "BlasterComponents/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/BlasterHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Sound/SoundCue.h"
#include "Weapon/Weapon.h"
#include "Weapon/WeaponTypes.h"

#define TARGETING_RANGE 80000.f

static const FName RightHandSocketName("RightHandSocket");

UCombatComponent::UCombatComponent()
{
    // Enable tick so we can see the temporary debug trace
    PrimaryComponentTick.bCanEverTick = true;
    InitializeCarriedAmmo();
}

void UCombatComponent::InitDefaultFOV()
{
    if (Character && Character->GetFollowCamera())
    {
        CurrentFOV = DefaultFOV = Character->GetFollowCamera()->FieldOfView;
    }
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    MirrorWalkSpeedBasedOnState();
    InitDefaultFOV();
    if (Character->HasAuthority())
    {
        InitializeCarriedAmmo();
    }
    bCanFire = true;
}

void UCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    bCanFire = true;
    GetWorld()->GetTimerManager().ClearTimer(FireTimer);
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

void UCombatComponent::PlayEquipSound() const
{
    if (const auto& Sound = EquippedWeapon->GetEquipSound())
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sound, Character->GetActorLocation());
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UCombatComponent::OnRep_EquippedWeapon()
{
    if (IsValid(EquippedWeapon) && IsValid(Character))
    {
        // Because EquippedWeapon, EquippedWeapon->WeaponState and Actor attachment are replicated variables
        // and may be replicated separately, it is possible that EquippedWeapon replicates before WeaponState
        // has been applied. This would result in attachment failing as state needs to be set in
        // EquippedWeapon->WeaponState before attachment can succeed. So we fake it and also apply it on clientside
        EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
        if (const auto HandSocket = Character->GetMesh()->GetSocketByName(RightHandSocketName))
        {
            HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
        }
        StopOrientingRotationToMovement();

        PlayEquipSound();
    }
}

void UCombatComponent::Fire()
{
    verify(Character->IsLocallyControlled());

    if (CanFire())
    {
        // Send fire action to the server
        ServerFire(HitTarget);

        if (EquippedWeapon)
        {
            bCanFire = false;
            CrosshairShootingFactor += 0.75f;
            CrosshairShootingFactor = FMath::Min(CrosshairShootingFactor, 1.f);

            StartFireTimer();
        }
    }
}

void UCombatComponent::SetFireButtonPressed(const bool bInFireButtonPressed)
{
    bFireButtonPressed = bInFireButtonPressed;
    if (bFireButtonPressed)
    {
        Fire();
    }
}

void UCombatComponent::TraceUnderCrossHairs(FHitResult& OutHitResult)
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

            // The distance from the center of the Viewport to the center of the Character
            const float DistanceToCharacter = (Character->GetActorLocation() - CrosshairWorldPosition).Size();

            // Push the start vector from the center of the viewport forward past the character
            // Starting the trace in front of the character means we can not trace and hit self nor hit another
            // character that is between the camera and the character (which would cause the gun to point in
            // reverse direction and fire bullets backwards which is not what we want.
            constexpr float UnitsInFrontOfCharacter = 100.f;
            const FVector Start{ CrosshairWorldPosition
                                 + (CrosshairWorldDirection * (DistanceToCharacter + UnitsInFrontOfCharacter)) };

            // Create the end by starting at start and moving along the unit vector direction our TARGETING_RANGE
            const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * TARGETING_RANGE };

            GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_Visibility);

            if (OutHitResult.GetActor() && OutHitResult.GetActor()->Implements<UInterfaceWithCrosshair>())
            {
                HUDPackage.CrosshairColor = FLinearColor::Red;
            }
            else
            {
                HUDPackage.CrosshairColor = FLinearColor::White;
            }

            if (!OutHitResult.bBlockingHit)
            {
                // If the trace does not hit anything then we set the OutHitResult.ImpactPoint to the end of our
                // trace As we know our caller is mostly just going to access this. Bad coding practice ... but such
                // is life when following tutorials
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

                if (bAiming)
                {
                    constexpr float Target = 0.f;
                    CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, Target, DeltaTime, 30.f);
                }
                else
                {
                    // If we are not aiming then we have a minimum spread
                    constexpr float Target = 0.58f;
                    CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, Target, DeltaTime, 30.f);
                }

                CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 30.f);

                HUDPackage.CrosshairSpread =
                    CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairAimFactor + CrosshairShootingFactor;
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
    // We can not interrupt a reload animation otherwise we will not get the notify that calls
    // FinishReloading() which will continue firing (and also update state etc). So we will get
    // permanently locked in reloading state which is bad
    if (Character && EquippedWeapon && ECombatState::Unoccupied == CombatState)
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

    if (Character && Character->IsLocallyControlled())
    {
        FHitResult HitResult;
        TraceUnderCrossHairs(HitResult);
        // We calculate this every frame so we can orient right hand to face where we are aiming
        HitTarget = HitResult.ImpactPoint;

        // Only need to update crosshairs for local clients
        SetHUDCrosshairs(DeltaTime);

        // Only need to update FOV local clients
        UpdateFOV(DeltaTime);
    }
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCombatComponent, EquippedWeapon);
    DOREPLIFETIME(UCombatComponent, bAiming);
    DOREPLIFETIME(UCombatComponent, CombatState);
    DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

void UCombatComponent::UpdateFOV(const float DeltaTime)
{
    check(Character && Character->IsLocallyControlled());
    if (EquippedWeapon && bAiming)
    {
        CurrentFOV = FMath::FInterpTo(CurrentFOV,
                                      EquippedWeapon->GetZoomedFOV(),
                                      DeltaTime,
                                      EquippedWeapon->GetZoomInterpSpeed());
    }
    else
    {
        CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
    }
    if (const auto CameraComponent = Character->GetFollowCamera(); ensure(CameraComponent))
    {
        CameraComponent->SetFieldOfView(CurrentFOV);
    }
}

void UCombatComponent::StopOrientingRotationToMovement() const
{
    Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::StartFireTimer()
{
    check(EquippedWeapon);
    check(Character);
    GetWorld()->GetTimerManager().SetTimer(FireTimer,
                                           this,
                                           &UCombatComponent::FireTimerFinished,
                                           EquippedWeapon->GetFireDelay());
}

void UCombatComponent::FireTimerFinished()
{
    bCanFire = true;
    if (EquippedWeapon)
    {
        if (bFireButtonPressed && EquippedWeapon->IsAutomaticFire())
        {
            Fire();
        }
        ReloadIfEmpty();
    }
}

bool UCombatComponent::CanFire() const
{
    // bCanFire is true when automatic delay has been satisfied
    // Can not fire while reloading
    // Can not fire if we do not have a weapon with ammo
    return bCanFire && ECombatState::Unoccupied == CombatState && EquippedWeapon && EquippedWeapon->HasAmmo();
}

void UCombatComponent::UpdateHUDCarriedAmmo()
{
    if (UNLIKELY(!Controller))
    {
        Controller = Cast<ABlasterPlayerController>(Character->Controller);
    }
    if (LIKELY(Controller))
    {
        if (Controller->IsLocalController())
        {
            Controller->SetHUDCarriedAmmo(CarriedAmmo);
        }
    }
}

void UCombatComponent::OnRep_CarriedAmmo()
{
    UpdateHUDCarriedAmmo();
}

void UCombatComponent::InitializeCarriedAmmo()
{
    CarriedAmmoMap.Add(EWeaponType::AssaultRifle, InitialAssaultRifleAmmo);
    CarriedAmmoMap.Add(EWeaponType::RocketLauncher, InitialRocketAmmo);
    CarriedAmmoMap.Add(EWeaponType::Pistol, InitialPistolAmmo);
    CarriedAmmoMap.Add(EWeaponType::SubmachineGun, InitialSubmachineGunAmmo);
}

int32 UCombatComponent::AmmoSlotsToReload()
{
    check(EquippedWeapon);

    const int32 EmptyAmmoSlotsInWeapon = EquippedWeapon->GetMaxAmmoCapacity() - EquippedWeapon->GetAmmo();

    if (const EWeaponType WeaponType = EquippedWeapon->GetWeaponType(); CarriedAmmoMap.Contains(WeaponType))
    {
        const int32 AmmoAvailable = CarriedAmmoMap[WeaponType];
        const int32 AmmoToAdd = FMath::Min(EmptyAmmoSlotsInWeapon, AmmoAvailable);
        // Perform a clamp ... just to ensure valid value if MaxAmmoCapacity is configured poorly
        return FMath::Clamp(EmptyAmmoSlotsInWeapon, 0, AmmoToAdd);
    }
    else
    {
        return 0;
    }
}

void UCombatComponent::OnRep_CombatState()
{
    if (Character)
    {
        check(!Character->HasAuthority());
        if (ECombatState::Reloading == CombatState)
        {
            // Trigger reload on client
            HandleReload();
        }
        else if (ECombatState::Unoccupied == CombatState)
        {
            // If we have finish reloading and we are still
            // holding the fire button then keep firing
            if (bFireButtonPressed)
            {
                Fire();
            }
        }
    }
}

void UCombatComponent::HandleReload() const
{
    if (Character)
    {
        Character->PlayReloadMontage();
    }
}

void UCombatComponent::ServerReload_Implementation()
{
    if (Character && EquippedWeapon)
    {
        CombatState = ECombatState::Reloading;
        // Trigger reload on server
        HandleReload();
    }
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
    if (IsValid(Character) && IsValid(WeaponToEquip))
    {
        if (EquippedWeapon)
        {
            // If we are holding a weapon and try to pick up another then drop
            // current weapon and store the current Ammo in map
            CarriedAmmoMap[EquippedWeapon->GetWeaponType()] = CarriedAmmo;
            EquippedWeapon->Dropped();
        }
        EquippedWeapon = WeaponToEquip;
        EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
        if (const auto HandSocket = Character->GetMesh()->GetSocketByName(RightHandSocketName))
        {
            HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
        }
        // Make sure we set owner of weapon so that weapon will be replicated wih player pawn
        EquippedWeapon->SetOwner(Character);
        EquippedWeapon->UpdateHUDAmmo();
        EquippedWeapon->ShowPickupWidget(false);

        const EWeaponType WeaponType = EquippedWeapon->GetWeaponType();
        CarriedAmmo = CarriedAmmoMap.Contains(WeaponType) ? CarriedAmmoMap[WeaponType] : 0;
        UpdateHUDCarriedAmmo();

        StopOrientingRotationToMovement();
        PlayEquipSound();
        ReloadIfEmpty();
    }
}

void UCombatComponent::Reload()
{
    // Check CarriedAmmo so we are not spamming server with RPC when not necessary
    // We also can not reload while reload in progress
    if (CarriedAmmo > 0 && ECombatState::Reloading != CombatState)
    {
        ServerReload();
    }
}

void UCombatComponent::ReloadIfEmpty()
{
    check(EquippedWeapon);
    if (!EquippedWeapon->HasAmmo())
    {
        // If we pick up an empty weapon then try to reload
        Reload();
    }
}

void UCombatComponent::FinishReloading()
{
    check(EquippedWeapon);
    // Only change state on the server and wait for it to be replicated to the client
    if (Character && Character->HasAuthority())
    {
        CombatState = ECombatState::Unoccupied;
        if (const int32 SlotsToReload = AmmoSlotsToReload(); SlotsToReload > 0)
        {
            const EWeaponType WeaponType = EquippedWeapon->GetWeaponType();
            check(CarriedAmmoMap.Contains(WeaponType));
            CarriedAmmoMap[WeaponType] -= SlotsToReload;
            // Update cached value as well
            CarriedAmmo = CarriedAmmoMap[WeaponType];
            UpdateHUDCarriedAmmo();
            EquippedWeapon->AddAmmo(SlotsToReload);
        }
    }
    if (bFireButtonPressed)
    {
        Fire();
    }
}
