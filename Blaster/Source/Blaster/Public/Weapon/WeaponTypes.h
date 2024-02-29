#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    AssaultRifle UMETA(DisplayName = "Assault Rifle"),
    RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
    Pistol UMETA(DisplayName = "Pistol")
};
