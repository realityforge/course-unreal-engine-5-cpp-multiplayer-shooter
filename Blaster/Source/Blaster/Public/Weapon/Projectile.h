#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class USoundCue;
class UProjectileMovementComponent;
class UBoxComponent;

UCLASS(Abstract)
class BLASTER_API AProjectile : public AActor
{
    GENERATED_BODY()

public:
    AProjectile();
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent,
                       AActor* OtherActor,
                       UPrimitiveComponent* OtherComponent,
                       FVector NormalImpulse,
                       const FHitResult& Hit);

private:
    UPROPERTY(EditAnywhere)
    UBoxComponent* CollisionBox{ nullptr };

    UPROPERTY(VisibleAnywhere)
    UProjectileMovementComponent* ProjectileMovementComponent{ nullptr };

    UPROPERTY(EditAnywhere, meta = (RuleRangerRequired))
    UParticleSystem* Tracer{ nullptr };

    UPROPERTY(VisibleAnywhere)
    UParticleSystemComponent* TracerComponent{ nullptr };

    UPROPERTY(EditAnywhere, meta = (RuleRangerRequired))
    UParticleSystem* ImpactParticles{ nullptr };

    UPROPERTY(EditAnywhere, meta = (RuleRangerRequired))
    USoundCue* ImpactSound{ nullptr };

    UPROPERTY(EditAnywhere)
    float Damage{ 20.f };

public:
    FORCEINLINE float GetDamage() const { return Damage; }
};
