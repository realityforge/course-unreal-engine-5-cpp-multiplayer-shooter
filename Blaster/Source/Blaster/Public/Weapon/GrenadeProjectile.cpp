#include "Weapon/GrenadeProjectile.h"
#include "BlasterLogging.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AGrenadeProjectile::AGrenadeProjectile()
{
    CreateProjectileMesh();

    ProjectileMovementComponent =
        CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->SetIsReplicated(true);
    ProjectileMovementComponent->bShouldBounce = true;
}

void AGrenadeProjectile::BeginPlay()
{
    Super::BeginPlay();

    SpawnTrailSystem();
    StartDestroyTimer();

    // Hook into callback so that we can make sounds on bounce.
    ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AGrenadeProjectile::OnBounce);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AGrenadeProjectile::OnBounce([[maybe_unused]] const FHitResult& ImpactResult,
                                  [[maybe_unused]] const FVector& ImpactVelocity)
{
    if (BounceSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
    }
}

void AGrenadeProjectile::OnHit(UPrimitiveComponent* HitComp,
                               AActor* OtherActor,
                               UPrimitiveComponent* OtherComp,
                               FVector NormalImpulse,
                               const FHitResult& Hit)
{
    BL_ULOG_WARNING("AGrenadeProjectile::OnHit");
    // Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AGrenadeProjectile::Destroyed()
{
    // Make sure it goes boom not wimper!
    ExplodeDamage();

    Super::Destroyed();
}
