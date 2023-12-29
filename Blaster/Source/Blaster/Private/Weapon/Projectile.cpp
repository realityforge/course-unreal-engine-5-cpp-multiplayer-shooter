#include "Weapon/Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

AProjectile::AProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    // Server authoritative but replicated to client
    bReplicates = true;

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
    if (Tracer)
    {
        // Create a particle system and attach it to a component so it will under component
        TracerComponent = UGameplayStatics::SpawnEmitterAttached(
            /* Particle System Template */ Tracer,
            /* Component to attach to */ CollisionBox,
            /* Name of bone/socket to attach to (none in our case) */ FName(),
            /* Initial location of particle system */ GetActorLocation(),
            /* Initial orientation of particle system */ GetActorRotation(),
            /* Calculate relative transform such that particle system  maintains the same world transform */
            EAttachLocation::KeepWorldPosition);
    }
    if (HasAuthority())
    {
        // We only run collision  on server so only add listener here
        // We also can not be added in the constructor as this sometimes does not work (Due to saving it to CDO?)
        CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
    }
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent,
                        AActor* OtherActor,
                        UPrimitiveComponent* OtherComponent,
                        FVector NormalImpulse,
                        const FHitResult& Hit)
{
    // Mark this object as destroyed at the end of the tick
    // This will create impact particles/sound in destroy action which runs on all clients as entity replicated
    // This avoids the need to replicate the rpc to create impact sounds/effects and then destroy message
    // This reduces the amount of messages by one
    Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AProjectile::Destroyed()
{
    Super::Destroyed();

    if (ImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
    }
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
    }
}