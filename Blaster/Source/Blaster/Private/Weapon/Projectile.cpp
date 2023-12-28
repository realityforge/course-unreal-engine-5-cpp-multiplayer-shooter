#include "Weapon/Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectile::AProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>("CollisionBox");
    SetRootComponent(CollisionBox);
    CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
    // We don't need a SetupAttachment here

    // this projectile will have its rotation updated each frame to match the direction of its velocity
    // Thus if we add fall-off due to gravity then the projectile will orient towards the direction of movement
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->InitialSpeed = 15000.f;
    ProjectileMovementComponent->MaxSpeed = 15000.f;
}

void AProjectile::BeginPlay()
{
    Super::BeginPlay();
}

void AProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
