#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "GrenadeProjectile.generated.h"

class UProjectileMovementComponent;

UCLASS(Abstract)
class BLASTER_API AGrenadeProjectile final : public AProjectile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TObjectPtr<USoundCue> BounceSound{ nullptr };

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent{ nullptr };

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

    virtual void OnHit(UPrimitiveComponent* HitComp,
                       AActor* OtherActor,
                       UPrimitiveComponent* OtherComp,
                       FVector NormalImpulse,
                       const FHitResult& Hit) override;

public:
    AGrenadeProjectile();

    virtual void Destroyed() override;
};
