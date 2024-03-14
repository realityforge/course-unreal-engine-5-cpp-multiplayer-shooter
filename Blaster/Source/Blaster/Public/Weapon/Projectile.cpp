#include "Weapon/Projectile.h"
#include "Blaster/Blaster.h"
#include "BlasterLogging.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystemInstanceController.h"
#include "Sound/SoundCue.h"

static FName ProjectileMeshComponentName(TEXT("ProjectileMesh"));

void AProjectile::CreateProjectileMesh()
{
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(ProjectileMeshComponentName);
    ProjectileMesh->SetupAttachment(RootComponent);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

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
    // Use ECC_SkeletalMesh over ECC_Pawn as ECC_Pawn would collide with the capsule which
    // would result in less precise hit detection. This traces against the Physics asset
    // associated with the mesh which is much more precise
    CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);
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
        // We only run collision on server so only add listener here
        // We also can not be added in the constructor as this sometimes does not work (Due to saving it to CDO?)
        CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);

        // Ignore the owner so that it does not explode on the owner ...
        // This kind of assumes you can not move faster than projectile and
        // get in front of it in any reasonable scenario
        // TODO: Apparently this is not the way
        // CollisionBox->IgnoreActorWhenMoving(Owner, true);
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

void AProjectile::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AProjectile::EmitDestroyCosmetics() const
{
    if (ImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
    }
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
    }
}

void AProjectile::SpawnTrailSystem()
{
    if (TrailSystem)
    {
        TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem,
                                                                            GetRootComponent(),
                                                                            FName(),
                                                                            GetActorLocation(),
                                                                            GetActorRotation(),
                                                                            EAttachLocation::KeepWorldPosition,
                                                                            false);
    }
}

void AProjectile::DeactivateTrailSystem() const
{
    if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
    {
        TrailSystemComponent->GetSystemInstanceController()->Deactivate();
    }
}

void AProjectile::ExplodeDamage()
{
    if (HasAuthority())
    {
        AController* InstigatorController{ nullptr };
        if (const auto& DamageInstigator = GetInstigator())
        {
            InstigatorController = DamageInstigator->GetController();
        }
        if (!InstigatorController)
        {
            BL_ULOG_WARNING("Unable to determine InstigatorController from Instigator '%s'",
                            GetInstigator() ? *GetInstigator()->GetName() : TEXT("?"))
        }
        UGameplayStatics::ApplyRadialDamageWithFalloff(this,                       // World context object
                                                       GetDamage(),                // BaseDamage
                                                       10.f,                       // MinimumDamage
                                                       GetActorLocation(),         // Origin
                                                       DamageInnerRadius,          // DamageInnerRadius
                                                       DamageOuterRadius,          // DamageOuterRadius
                                                       1.f,                        // DamageFalloff
                                                       UDamageType::StaticClass(), // DamageTypeClass
                                                       TArray<AActor*>(),          // IgnoreActors
                                                       this,                       // DamageCauser
                                                       InstigatorController        // InstigatorController
        );
    }
}

void AProjectile::StartDestroyTimer()
{
    GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AProjectile::DestroyTimerFinished, DestroyAfterDuration);
}

void AProjectile::ClearDestroyTimer()
{
    // Clear the timer just in case we were destroyed for another reason
    GetWorldTimerManager().ClearTimer(DestroyTimerHandle);
}

void AProjectile::DestroyTimerFinished()
{
    // Destroy self. Although why don't we just schedule AActor::Destroy in timer?
    Destroy();
}

void AProjectile::Destroyed()
{
    Super::Destroyed();

    EmitDestroyCosmetics();
}
