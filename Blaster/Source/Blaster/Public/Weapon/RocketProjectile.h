#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "RocketProjectile.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(Abstract)
class BLASTER_API ARocketProjectile : public AProjectile
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> RocketMesh{ nullptr };

    FTimerHandle DestroyTimerHandle;

    /** Destroy the actor after this duration. */
    UPROPERTY(EditDefaultsOnly)
    float DestroyAfterDuration{ 3.f };

    /** Component created at runtime if TrailSystem is not null. */
    UPROPERTY()
    TObjectPtr<UNiagaraComponent> TrailSystemComponent{ nullptr };

    /** FX representing projectile trail. */
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UNiagaraSystem> TrailSystem{ nullptr };

    /** Component created at runtime if ProjectileLoop and LoopingSoundAttenuation are not null. */
    UPROPERTY()
    TObjectPtr<UAudioComponent> ProjectileLoopComponent{ nullptr };

    /** Sound that loops while projectile is in the air. */
    UPROPERTY(EditDefaultsOnly, meta = (RuleRangerRequiredIf = LoopingSoundAttenuation))
    TObjectPtr<USoundCue> ProjectileLoop{ nullptr };

    /** Sound attenuation for looping sound. */
    UPROPERTY(EditDefaultsOnly, meta = (RuleRangerRequiredIf = ProjectileLoop))
    TObjectPtr<USoundAttenuation> LoopingSoundAttenuation{ nullptr };

    void DestroyTimerFinished();

protected:
    virtual void BeginPlay() override;

    virtual void OnHit(UPrimitiveComponent* HitComp,
                       AActor* OtherActor,
                       UPrimitiveComponent* OtherComp,
                       FVector NormalImpulse,
                       const FHitResult& Hit) override;

public:
    ARocketProjectile();

    virtual void Destroyed() override;
};
