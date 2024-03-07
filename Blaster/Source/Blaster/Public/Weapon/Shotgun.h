#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

UCLASS(Abstract)
class BLASTER_API AShotgun final : public AHitScanWeapon
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Scatter")
    uint32 NumberOfPellets{ 10 };

public:
    virtual void Fire(const FVector& HitTarget) override;
};
