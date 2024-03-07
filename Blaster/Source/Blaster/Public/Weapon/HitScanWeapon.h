#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "HitScanWeapon.generated.h"

UCLASS(Abstract)
class BLASTER_API AHitScanWeapon final : public AWeapon
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    float Damage{ 20.f };

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UParticleSystem> ImpactParticles{ nullptr };

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UParticleSystem> BeamParticles{ nullptr };

    /** Particle emitted on fire. Prefer Notify in FireAnimation but can use this if no animation available. */
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UParticleSystem> MuzzleFlash{ nullptr };

    /** Sound played on fire. Prefer Notify in FireAnimation but can use this if no animation available. */
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<USoundCue> FireSound{ nullptr };

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<USoundCue> HitSound{ nullptr };

public:
    virtual void Fire(const FVector& HitTarget) override;
};
