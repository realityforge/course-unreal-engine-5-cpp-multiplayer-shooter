#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "ProjectileWeapon.generated.h"

class AProjectile;

UCLASS(Abstract)
class BLASTER_API AProjectileWeapon final : public AWeapon
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, meta = (AllowAbstract = "false", RuleRangerRequired))
    TSubclassOf<AProjectile> ProjectileClass{ nullptr };

public:
    virtual void Fire(const FVector& HitTarget) override;
};
