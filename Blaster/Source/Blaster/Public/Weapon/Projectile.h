#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USoundCue;
class UProjectileMovementComponent;
class UBoxComponent;

UCLASS(Abstract)
class BLASTER_API AProjectile : public AActor
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> ProjectileMesh{ nullptr };

    UPROPERTY(EditAnywhere)
    TObjectPtr<UBoxComponent> CollisionBox{ nullptr };

    UPROPERTY(EditAnywhere)
    TObjectPtr<UParticleSystem> Tracer{ nullptr };

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UParticleSystemComponent> TracerComponent{ nullptr };

    UPROPERTY(EditAnywhere, meta = (RuleRangerRequired))
    TObjectPtr<UParticleSystem> ImpactParticles{ nullptr };

    UPROPERTY(EditAnywhere, meta = (RuleRangerRequired))
    TObjectPtr<USoundCue> ImpactSound{ nullptr };

    UPROPERTY(EditAnywhere)
    float Damage{ 20.f };

    /** Component created at runtime if TrailSystem is not null. */
    UPROPERTY()
    TObjectPtr<UNiagaraComponent> TrailSystemComponent{ nullptr };

    /** FX representing projectile trail. */
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UNiagaraSystem> TrailSystem{ nullptr };

    UPROPERTY(EditDefaultsOnly)
    float DamageInnerRadius{ 200.f };

    UPROPERTY(EditDefaultsOnly)
    float DamageOuterRadius{ 500.f };

    FTimerHandle DestroyTimerHandle;

    /** Destroy the actor after this duration. */
    UPROPERTY(EditDefaultsOnly)
    float DestroyAfterDuration{ 3.f };

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent,
                       AActor* OtherActor,
                       UPrimitiveComponent* OtherComponent,
                       FVector NormalImpulse,
                       const FHitResult& Hit);

    FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
    FORCEINLINE UParticleSystem* GetImpactParticles() const { return ImpactParticles; }
    FORCEINLINE USoundCue* GetImpactSound() const { return ImpactSound; }
    FORCEINLINE UStaticMeshComponent* GetProjectileMesh() const { return ProjectileMesh; }

    void EmitDestroyCosmetics() const;

    /**
     * Spawn the trail system attached to the projectile if a TrailSystem is configured.
     */
    void SpawnTrailSystem();

    /**
     *  Deactivate the trail system that was created via SpawnTrailSystem.
     */
    void DeactivateTrailSystem() const;

    /** Apply damage around projectile. */
    void ExplodeDamage();

    void StartDestroyTimer();
    void ClearDestroyTimer();

    void DestroyTimerFinished();

    /** Method to create the Projectile Mesh. Should only be invoked in constructor. */
    void CreateProjectileMesh();

public:
    AProjectile();
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;

    FORCEINLINE float GetDamage() const { return Damage; }
};
