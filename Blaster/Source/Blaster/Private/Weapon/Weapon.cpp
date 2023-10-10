#include "Weapon/Weapon.h"
#include "Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

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
    if (PickupWidget)
    {
        PickupWidget->SetVisibility(false);
    }
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

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWeapon::ShowPickupWidget(const bool bShowWidget)
{
    if (PickupWidget)
    {
        PickupWidget->SetVisibility(bShowWidget);
    }
}
