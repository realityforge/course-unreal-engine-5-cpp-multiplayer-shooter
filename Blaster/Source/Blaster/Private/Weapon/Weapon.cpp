#include "Weapon/Weapon.h"
#include "Casing.h"
#include "Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = false;

    // Mark the weapon as replicated
    bReplicates = true;

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh);

    WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
    WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
    AreaSphere->SetupAttachment(WeaponMesh);
    AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
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
    }
}

void AWeapon::OnRep_WeaponState() const
{
    ensure(!HasAuthority());
    OnWeaponStateUpdated();
}

void AWeapon::SetWeaponState(const EWeaponState InWeaponState)
{
    ensure(HasAuthority());
    WeaponState = InWeaponState;
    // This is only required on the server
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
}

void AWeapon::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::ShowPickupWidget(const bool bShowWidget) const
{
    if (IsValid(PickupWidget))
    {
        PickupWidget->SetVisibility(bShowWidget);
    }
}
