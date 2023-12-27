#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "ProjectileWeapon.generated.h"

class AProjectile;

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, meta = (AllowAbstract = "false", RuleRangerRequired))
    TSubclassOf<AProjectile> ProjectileClass;

public:
    virtual void Fire(const FVector& HitTarget) override;
};
