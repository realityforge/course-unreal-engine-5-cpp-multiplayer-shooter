#include "Weapon/RocketProjectile.h"
#include "BlasterLogging.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystemInstanceController.h"
#include "Sound/SoundCue.h"
#include "Weapon/RocketMovementComponent.h"

static FName RocketMeshComponentName(TEXT("RocketMesh"));
static FName RocketMovementComponentName(TEXT("RocketMovementComponent"));

ARocketProjectile::ARocketProjectile()
{
    RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(RocketMeshComponentName);
    RocketMesh->SetupAttachment(RootComponent);
    RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(RocketMovementComponentName);
    RocketMovementComponent->bRotationFollowsVelocity = true;
    RocketMovementComponent->InitialSpeed = 1500.f;
    RocketMovementComponent->MaxSpeed = 1500.f;
    RocketMovementComponent->ProjectileGravityScale = 0.f;
    RocketMovementComponent->SetIsReplicated(true);
}

void ARocketProjectile::DestroyTimerFinished()
{
    // Destroy self. Although why don't we just schedule AActor::Destroy in timer?
    Destroy();
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
                                                       200.f,                      // DamageInnerRadius
                                                       500.f,                      // DamageOuterRadius
                                                       1.f,                        // DamageFalloff
                                                       UDamageType::StaticClass(), // DamageTypeClass
                                                       TArray<AActor*>(),          // IgnoreActors
                                                       this,                       // DamageCauser
                                                       InstigatorController        // InstigatorController
        );
    }

    // We do not call destroy here ... because it would instantly destroy all the components which include the
    // trail emitter component and the sound ... so instead we do the following hackery and delay destroy action

    GetWorldTimerManager().SetTimer(DestroyTimerHandle,
                                    this,
                                    &ARocketProjectile::DestroyTimerFinished,
                                    DestroyAfterDuration);

    // Calling this here seems wrong. It should somehow be signalled from the server....
    // Instead we emit here and hide the mesh ... even if the hit would not have
    // occurred on the server
    EmitDestroyCosmetics();

    if (RocketMesh)
    {
        RocketMesh->SetVisibility(false);
    }
    if (GetCollisionBox())
    {
        GetCollisionBox()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    // Stop the niagara system on impact ...
    // (shouldn't we just stop the flame emitter?)
    if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
    {
        TrailSystemComponent->GetSystemInstanceController()->Deactivate();
    }
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
    GetWorldTimerManager().ClearTimer(DestroyTimerHandle);
}
