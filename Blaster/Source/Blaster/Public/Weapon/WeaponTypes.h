#pragma once

#include "CoreMinimal.h"

/**
 * The range out to which we can target.
 *
 * Used as range when tracing hits.
 */
#define TARGETING_RANGE 80000.f

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    AssaultRifle UMETA(DisplayName = "Assault Rifle"),
    RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
    Pistol UMETA(DisplayName = "Pistol"),
    SubmachineGun UMETA(DisplayName = "Submachine Gun"),
    Shotgun UMETA(DisplayName = "Shotgun"),
    SniperRifle UMETA(DisplayName = "Sniper Rifle"),
    GrenadeLauncher UMETA(DisplayName = "Grenade Launcher")
};
