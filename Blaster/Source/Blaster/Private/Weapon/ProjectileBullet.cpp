#include "Weapon/ProjectileBullet.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent,
                              AActor* OtherActor,
                              UPrimitiveComponent* OtherComponent,
                              FVector NormalImpulse,
                              const FHitResult& Hit)
{
    // Owner chain gos from projectile to weapon to character using weapon
    if (const auto OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        if (AController* OwnerController = OwnerCharacter->Controller)
        {
            UGameplayStatics::ApplyDamage(OtherActor, GetDamage(), OwnerController, this, UDamageType::StaticClass());
        }
    }
    Super::OnHit(HitComponent, OtherActor, OtherComponent, NormalImpulse, Hit);
}

AProjectileBullet::AProjectileBullet()
{
    static const FName ProjectileMovementComponentName(TEXT("ProjectileMovementComponent"));

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(ProjectileMovementComponentName);
    // We don't need a SetupAttachment here

    // this projectile will have its rotation updated each frame to match the direction of its velocity
    // Thus if we add fall-off due to gravity then the projectile will orient towards the direction of movement
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->InitialSpeed = 15000.f;
    ProjectileMovementComponent->MaxSpeed = 15000.f;
    ProjectileMovementComponent->ProjectileGravityScale = 0.f;
    ProjectileMovementComponent->SetIsReplicated(true);
}
