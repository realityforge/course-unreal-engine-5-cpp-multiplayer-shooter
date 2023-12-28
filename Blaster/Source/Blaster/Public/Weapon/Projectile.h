#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UProjectileMovementComponent;
class UBoxComponent;

UCLASS(Abstract)
class BLASTER_API AProjectile : public AActor
{
    GENERATED_BODY()

public:
    AProjectile();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere)
    UBoxComponent* CollisionBox{ nullptr };

    UPROPERTY(VisibleAnywhere)
    UProjectileMovementComponent* ProjectileMovementComponent{ nullptr };

    UPROPERTY(EditAnywhere)
    UParticleSystem* Tracer{ nullptr };

    UPROPERTY(VisibleAnywhere)
    UParticleSystemComponent* TracerComponent;
};
