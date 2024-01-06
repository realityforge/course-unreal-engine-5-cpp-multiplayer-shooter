#pragma once

#include "CoreMinimal.h"

// Create a custom collision Channel so can specifically collide with mesh of characters for more accurate hits
// (If we use ECC_Pawn then we would also collide with Capsule of Character which is less precise)
#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1
