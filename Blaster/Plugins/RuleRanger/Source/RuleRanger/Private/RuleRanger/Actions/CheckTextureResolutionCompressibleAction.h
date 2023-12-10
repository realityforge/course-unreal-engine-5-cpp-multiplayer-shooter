/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RuleRangerAction.h"
#include "CheckTextureResolutionCompressibleAction.generated.h"

UENUM()
enum class ETextureDivisor : uint8
{
    TD_Four UMETA(DisplayString = "Divisible by 4"),
    TD_Eight UMETA(DisplayString = "Divisible by 8"),
    TD_Twelve UMETA(DisplayString = "Divisible by 12")
};

/**
 * Action to check that a Texture has dimensions that are divisible by 4.
 * Other texture resolutions will not compress. Nearly all (if not every) game-engine compression format
 * (DXT, ETC, ASTC, etc.) is based on the idea of 4x4 blocks or sometimes 6x6 or 8x8. Bringing in an image
 * with dimensions that are a multiple of 4, like 304x304, results in DXT compression, but 305x305 gives
 * B8G8R8A8(uncompressed). So if you must have non-Po2 textures then the dimensions should be multiples
 * of 4 (or 8 or 12 to be really safe).
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"),
       Blueprintable,
       BlueprintType,
       CollapseCategories,
       DefaultToInstanced,
       EditInlineNew)
class RULERANGER_API UCheckTextureResolutionCompressibleAction final : public URuleRangerAction
{
    GENERATED_BODY()

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

private:
    /** The value by which texture should be divisible. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (AllowPrivateAccess, ExposeOnSpawn))
    ETextureDivisor TextureDivisor{ ETextureDivisor::TD_Four };
};
