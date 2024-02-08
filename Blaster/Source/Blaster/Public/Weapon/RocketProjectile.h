#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "RocketProjectile.generated.h"

UCLASS(Abstract)
class BLASTER_API ARocketProjectile : public AProjectile
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> RocketMesh;

protected:
    virtual void OnHit(UPrimitiveComponent* HitComp,
                       AActor* OtherActor,
                       UPrimitiveComponent* OtherComp,
                       FVector NormalImpulse,
                       const FHitResult& Hit) override;

public:
    ARocketProjectile();
};
