#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "RocketProjectile.generated.h"

class URocketMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(Abstract)
class BLASTER_API ARocketProjectile final : public AProjectile
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> RocketMesh{ nullptr };

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<URocketMovementComponent> RocketMovementComponent{ nullptr };

    /** Component created at runtime if ProjectileLoop and LoopingSoundAttenuation are not null. */
    UPROPERTY()
    TObjectPtr<UAudioComponent> ProjectileLoopComponent{ nullptr };

    /** Sound that loops while projectile is in the air. */
    UPROPERTY(EditDefaultsOnly, meta = (RuleRangerRequiredIf = LoopingSoundAttenuation))
    TObjectPtr<USoundCue> ProjectileLoop{ nullptr };

    /** Sound attenuation for looping sound. */
    UPROPERTY(EditDefaultsOnly, meta = (RuleRangerRequiredIf = ProjectileLoop))
    TObjectPtr<USoundAttenuation> LoopingSoundAttenuation{ nullptr };

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
