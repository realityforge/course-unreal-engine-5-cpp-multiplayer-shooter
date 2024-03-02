

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

    UPROPERTY(EditAnywhere)
    TObjectPtr<UParticleSystem> ImpactParticles;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UParticleSystem> BeamParticles;

public:
    virtual void Fire(const FVector& HitTarget) override;
};
