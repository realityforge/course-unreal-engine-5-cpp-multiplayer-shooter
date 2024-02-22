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

    UPROPERTY(EditAnywhere)
    TObjectPtr<UBoxComponent> CollisionBox{ nullptr };

    UPROPERTY(EditAnywhere)
    TObjectPtr<UParticleSystem> Tracer{ nullptr };

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UParticleSystemComponent> TracerComponent{ nullptr };

    UPROPERTY(EditAnywhere, meta = (RuleRangerRequired))
    TObjectPtr<UParticleSystem> ImpactParticles{ nullptr };

    UPROPERTY(EditAnywhere, meta = (RuleRangerRequired))
    TObjectPtr<USoundCue> ImpactSound{ nullptr };

    UPROPERTY(EditAnywhere)
    float Damage{ 20.f };

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent,
                       AActor* OtherActor,
                       UPrimitiveComponent* OtherComponent,
                       FVector NormalImpulse,
                       const FHitResult& Hit);

    FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
    FORCEINLINE UParticleSystem* GetImpactParticles() const { return ImpactParticles; }
    FORCEINLINE USoundCue* GetImpactSound() const { return ImpactSound; }

    void EmitDestroyCosmetics() const;

public:
    AProjectile();
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;

    FORCEINLINE float GetDamage() const { return Damage; }
};
