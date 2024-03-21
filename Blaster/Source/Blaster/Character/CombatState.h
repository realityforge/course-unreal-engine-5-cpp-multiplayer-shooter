#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Unoccupied UMETA(DisplayName = "Unoccupied"),
    Reloading UMETA(DisplayName = "Reloading"),
    ThrowingGrenade UMETA(DisplayName = "ThrowingGrenade"),
};
