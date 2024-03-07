#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "HitScanWeapon.generated.h"

UCLASS(Abstract)
class BLASTER_API AHitScanWeapon : public AWeapon
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

    /// -------------------------------------------------------------- ///
    /// Weapon Scatter
    ///
    /// Weapon scattering as shots being scattered within a sphere at
    /// a distance. So we randomize a point in the sphere and use that
    /// as a direction. The DistanceToSphere and SphereRadius just gives us
    /// "user friendly" way for the designer to scatter in a sphere.
    /// -------------------------------------------------------------- ///

    /** Should this weapon use scattering? */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon ~Scatter")
    bool bUseScatter{ false };

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Scatter")
    float DistanceToSphere{ 800.f };

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Scatter")
    float SphereRadius{ 75.f };

protected:
    FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget) const;
    void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit) const;

    FORCEINLINE UParticleSystem* GetImpactParticles() const { return ImpactParticles; };
    FORCEINLINE USoundCue* GetHitSound() const { return HitSound; };
    FORCEINLINE float GetDamage() const { return Damage; };

public:
    virtual void Fire(const FVector& HitTarget) override;
};
