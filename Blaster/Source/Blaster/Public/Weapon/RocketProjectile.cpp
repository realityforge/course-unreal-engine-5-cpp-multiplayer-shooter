#include "Weapon/RocketProjectile.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Weapon/RocketMovementComponent.h"

static FName RocketMovementComponentName(TEXT("RocketMovementComponent"));

ARocketProjectile::ARocketProjectile()
{
    CreateProjectileMesh();
    RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(RocketMovementComponentName);
    RocketMovementComponent->bRotationFollowsVelocity = true;
    RocketMovementComponent->InitialSpeed = 1500.f;
    RocketMovementComponent->MaxSpeed = 1500.f;
    RocketMovementComponent->ProjectileGravityScale = 0.f;
    RocketMovementComponent->SetIsReplicated(true);
}

void ARocketProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority())
    {
        // We already bind this delegate where HasAuthority() is true in superclass
        // We (stupidly) want to also call it on the client ... so we add the client scenario here

        // ReSharper disable once CppBoundToDelegateMethodIsNotMarkedAsUFunction
        // Suppress reason is that the superclass marks function as UFUNCTION()
        GetCollisionBox()->OnComponentHit.AddDynamic(this, &ARocketProjectile::OnHit);
    }

    SpawnTrailSystem();
    if (ProjectileLoop && LoopingSoundAttenuation)
    {
        ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(ProjectileLoop,
                                                                       GetRootComponent(),
                                                                       FName(),
                                                                       GetActorLocation(),
                                                                       EAttachLocation::KeepWorldPosition,
                                                                       false,
                                                                       1.f,
                                                                       1.f,
                                                                       0.f,
                                                                       LoopingSoundAttenuation,
                                                                       (USoundConcurrency*)nullptr,
                                                                       false);
    }
}

void ARocketProjectile::OnHit(UPrimitiveComponent* HitComp,
                              AActor* OtherActor,
                              UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse,
                              const FHitResult& Hit)
{
    if (GetOwner() == OtherActor)
    {
        // Ignore hits on owner.
        // This is only used to stop the rocket immediately exploding when fired because
        // the player was moving forward and now intercepts with collision box of the rocket.
        // This is an ugly hack that should be done via CollisionBox->IgnoreActorWhenMoving(Owner, true)
        // on Rocket .... or so I would think. Leaving this here as this is the way the tutorial implemented
        return;
    }
    ExplodeDamage();

    // We do not call destroy here ... because it would instantly destroy all the components which include the
    // trail emitter component and the sound ... so instead we do the following hackery and delay destroy action
    StartDestroyTimer();

    // Calling this here seems wrong. It should somehow be signalled from the server....
    // Instead we emit here and hide the mesh ... even if the hit would not have
    // occurred on the server
    EmitDestroyCosmetics();

    if (GetProjectileMesh())
    {
        GetProjectileMesh()->SetVisibility(false);
    }
    if (GetCollisionBox())
    {
        GetCollisionBox()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    // Stop the niagara system on impact ...
    // (shouldn't we just stop the flame emitter?)
    DeactivateTrailSystem();
    // Stop making sound on impacts
    if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
    {
        ProjectileLoopComponent->Stop();
    }
}

void ARocketProjectile::Destroyed()
{
    // Deliberately do not call super method as otherwise that will also play the sound and explosion again

    // Clear the timer just in case we were destroyed for another reason
    ClearDestroyTimer();
}
