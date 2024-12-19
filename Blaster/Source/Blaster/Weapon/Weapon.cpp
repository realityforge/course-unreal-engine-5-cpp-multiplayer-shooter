#include "Weapon/Weapon.h"
#include "BlasterCustomDepthStencilValue.h"
#include "Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Misc/DataValidation.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Weapon/Casing.h"

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = false;

    // Mark the weapon as replicated
    bReplicates = true;

    // ReSharper disable once CppVirtualFunctionCallInsideCtor
    SetReplicateMovement(true);

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh);

    WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
    WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    EnableCustomDepth(true);
    WeaponMesh->SetCustomDepthStencilValue(BlueOutline);
    // Need to mark the render state as dirty to get the stencil value updated
    // TODO: Is this really needed in a constructor??? (does constructor not just set up the default instance?)
    WeaponMesh->MarkRenderStateDirty();

    AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
    AreaSphere->SetupAttachment(WeaponMesh);
    AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
    PickupWidget->SetWidgetSpace(EWidgetSpace::Screen);
    PickupWidget->SetDrawAtDesiredSize(true);
    PickupWidget->SetupAttachment(WeaponMesh);
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        // We only enable the collision on the server
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnAreaSphereBeginOverlap);
        AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnAreaSphereEndOverlap);
    }
    ShowPickupWidget(false);
}

void AWeapon::OnAreaSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                       AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp,
                                       int32 OtherBodyIndex,
                                       bool bFromSweep,
                                       const FHitResult& SweepResult)
{
    if (const auto BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
    {
        BlasterCharacter->SetOverlappingWeapon(this);
    }
}

void AWeapon::OnAreaSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                     AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp,
                                     int32 OtherBodyIndex)
{
    if (const auto BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
    {
        BlasterCharacter->SetOverlappingWeapon(nullptr);
    }
}

void AWeapon::OnWeaponStateUpdated() const
{
    if (EWeaponState::EWS_Equipped == WeaponState)
    {
        ShowPickupWidget(false);
        WeaponMesh->SetSimulatePhysics(false);
        WeaponMesh->SetEnableGravity(false);
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        if (EWeaponType::SubmachineGun == WeaponType)
        {
            WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            WeaponMesh->SetEnableGravity(true);
            WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
        }
        EnableCustomDepth(false);
    }
    else if (EWeaponState::EWS_Dropped == WeaponState)
    {
        WeaponMesh->SetSimulatePhysics(true);
        WeaponMesh->SetEnableGravity(true);
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        if (EWeaponType::SubmachineGun == WeaponType)
        {
            // Make sure we revert the SMG collision response to defaults
            WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
            WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
        }
        EnableCustomDepth(true);
        WeaponMesh->SetCustomDepthStencilValue(BlueOutline);
        WeaponMesh->MarkRenderStateDirty();
    }
}

void AWeapon::OnRep_WeaponState() const
{
    ensure(!HasAuthority());
    OnWeaponStateUpdated();
}

void AWeapon::SetWeaponState(const EWeaponState InWeaponState)
{
    WeaponState = InWeaponState;
    if (HasAuthority())
    {
        // This is only required on the server as clientside effects handled inOnWeaponStateUpdated
        // We need this guard because we directly call from client in CombatClient.
        if (EWeaponState::EWS_Equipped == WeaponState)
        {
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        else if (EWeaponState::EWS_Dropped == WeaponState)
        {
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
    }
    OnWeaponStateUpdated();
}

void AWeapon::Fire(const FVector& HitTarget)
{
    if (IsValid(FireAnimation))
    {
        WeaponMesh->PlayAnimation(FireAnimation, false);
    }
    if (CasingClass)
    {
        if (const auto& Socket = GetWeaponMesh()->GetSocketByName(FName("AmmoEject")))
        {
            if (const auto World = GetWorld())
            {
                const auto SocketTransform = Socket->GetSocketTransform(WeaponMesh);
                World->SpawnActor<ACasing>(CasingClass,
                                           SocketTransform.GetLocation(),
                                           SocketTransform.GetRotation().Rotator());
            }
        }
    }
    UseAmmo();
}

void AWeapon::OnRep_Ammo()
{
    UpdateHUDAmmo();
    if (const auto BlasterOwnerCharacter = GetOwnerCharacter())
    {
        if (BlasterOwnerCharacter->GetCombat() && IsAmmoAtCapacity())
        {
            BlasterOwnerCharacter->GetCombat()->JumpToShotgunEnd();
        }
    }
}

void AWeapon::UseAmmo()
{
    Ammo = FMath::Clamp(Ammo - 1, 0, MaxAmmoCapacity);
    UpdateHUDAmmo();
}

ABlasterCharacter* AWeapon::GetOwnerCharacter()
{
    return OwnerCharacter ? OwnerCharacter : (OwnerCharacter = Cast<ABlasterCharacter>(GetOwner()));
}

ABlasterPlayerController* AWeapon::GetOwnerController()
{
    if (OwnerController)
    {
        return OwnerController;
    }
    else
    {
        if (const auto BlasterCharacter = GetOwnerCharacter(); BlasterCharacter && BlasterCharacter->Controller)
        {
            OwnerController = Cast<ABlasterPlayerController>(BlasterCharacter->Controller);
        }
        return OwnerController;
    }
}

void AWeapon::EnableCustomDepth(const bool bEnable) const
{
    if (WeaponMesh)
    {
        // Should we be writing custom depth/stencil buffer for special effects?
        WeaponMesh->SetRenderCustomDepth(bEnable);
    }
}

void AWeapon::UpdateHUDAmmo()
{
    if (const auto PlayerController = GetOwnerController())
    {
        // Initialize Health on HUD
        if (PlayerController->IsLocalController())
        {
            PlayerController->SetHUDWeaponAmmo(Ammo);
        }
    }
}

void AWeapon::ClearCachedOwnerProperties()
{
    // Clear the cached values from Owner as Owner has changed
    OwnerCharacter = nullptr;
    OwnerController = nullptr;
}

void AWeapon::OnRep_Owner()
{
    Super::OnRep_Owner();

    ClearCachedOwnerProperties();
    // If we have an owner, then the next call will recache and update HUD
    // otherwise out Owner is now nullptr and thus no HUD to update
    UpdateHUDAmmo();
}

#if WITH_EDITOR
EDataValidationResult AWeapon::IsDataValid(FDataValidationContext& Context) const
{
    if (!GetClass()->HasAnyClassFlags(CLASS_Abstract))
    {
        if (CasingClass && !GetWeaponMesh()->DoesSocketExist(FName("AmmoEject")))
        {
            Context.AddError(FText::FromString(
                FString::Printf(TEXT("The socket named 'AmmoEject' does not exist on the mesh named '%s' referenced "
                                     "from the 'WeaponMesh' component but the weapon has defined a CasingClass '%s' "
                                     "and thus will attempt to eject casing during firing. This requires the presence "
                                     "of the socket to indicate where the ammo is ejected."),
                                GetWeaponMesh()->GetSkeletalMeshAsset()
                                    ? *GetWeaponMesh()->GetSkeletalMeshAsset()->GetOutermostObject()->GetName()
                                    : TEXT("None"),
                                *CasingClass->GetOutermost()->GetName())));
        }
        if (!GetWeaponMesh()->DoesSocketExist(FName("LeftHandSocket")))
        {
            Context.AddError(FText::FromString(
                FString::Printf(TEXT("The socket named 'LeftHandSocket' does not exist on the mesh named '%s' "
                                     "referenced from the 'WeaponMesh' component. The socket is required to support "
                                     "the FABRIK system to move the leftHand to support the weapon."),
                                GetWeaponMesh()->GetSkeletalMeshAsset()
                                    ? *GetWeaponMesh()->GetSkeletalMeshAsset()->GetOutermostObject()->GetName()
                                    : TEXT("None"))));
        }
    }
    return Super::IsDataValid(Context);
}
#endif

void AWeapon::Dropped()
{
    // Changing the state will transform all the other state elements
    SetWeaponState(EWeaponState::EWS_Dropped);

    // It is unclear why these following lines are not in a state transition for dropped

    // Detach the weapon from the Hand socket
    WeaponMesh->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
    // Remove ownership from character so other characters can pickup
    SetOwner(nullptr);
    // Reset properties that are cached values derived from Owner
    // This happens on the server while OnRep_Owner() does the same on clients
    ClearCachedOwnerProperties();
}

void AWeapon::AddAmmo(const int32 InAmmo)
{
    check(InAmmo > 0);
    // Ignore excess Ammo
    Ammo = FMath::Clamp(Ammo + InAmmo, 0, MaxAmmoCapacity);
    UpdateHUDAmmo();
}

void AWeapon::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWeapon, WeaponState);
    DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::ShowPickupWidget(const bool bShowWidget) const
{
    if (IsValid(PickupWidget))
    {
        PickupWidget->SetVisibility(bShowWidget);
    }
}
