#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileBullet.generated.h"

class UProjectileMovementComponent;

UCLASS()
class BLASTER_API AProjectileBullet final : public AProjectile
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent{ nullptr };

protected:
    virtual void OnHit(UPrimitiveComponent* HitComponent,
                       AActor* OtherActor,
                       UPrimitiveComponent* OtherComponent,
                       FVector NormalImpulse,
                       const FHitResult& Hit) override;

public:
    AProjectileBullet();
};
