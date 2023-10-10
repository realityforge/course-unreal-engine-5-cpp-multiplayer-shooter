#include "Weapon/Weapon.h"
#include "Components/SphereComponent.h"

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
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        // We only enable the collission on the server
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
