

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
    TObjectPtr<UParticleSystem> ImpactParticles;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UParticleSystem> BeamParticles;

    /** Particle emitted on fire. Prefer Notify in FireAnimation but can use this if no animation available. */
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UParticleSystem> MuzzleFlash;

    /** Sound played on fire. Prefer Notify in FireAnimation but can use this if no animation available. */
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<USoundCue> FireSound;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<USoundCue> HitSound;

public:
    virtual void Fire(const FVector& HitTarget) override;
};
