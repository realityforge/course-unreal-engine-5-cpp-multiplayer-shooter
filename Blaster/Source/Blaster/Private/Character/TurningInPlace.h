#pragma once

#include "CoreMinimal.h"

/** Enum indicating whether the character is turning in place. */
UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
    TIP_NotTurning UMETA(DisplayString = "Not Turning"),
    TIP_TurningLeft UMETA(DisplayString = "Turning Left"),
    TIP_TurningRight UMETA(DisplayString = "Turning Right"),

    TIP_MAX UMETA(Hidden)
};
