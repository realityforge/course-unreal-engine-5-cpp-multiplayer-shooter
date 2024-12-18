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

#include "TextureResolutionConstraint.h"
#include "UObject/Object.h"
#include "RuleRangerTextureConvention.generated.h"

UENUM(BlueprintType)
enum class ERuleRangerTextureColorSpace : uint8
{
    Auto,
    Linear,
    SRGB UMETA(DisplayName = "sRGB")
};

/**
 * The structure defining conventions for different texture types.
 */
USTRUCT(BlueprintType)
struct FRuleRangerTextureConvention final : public FTableRowBase
{
    GENERATED_BODY();

    /** An explanation of the convention. */
    UPROPERTY(EditAnywhere)
    FString Description{ "" };

    /** The Suffix associated with this texture type. */
    UPROPERTY(EditAnywhere)
    FString Suffix{ "" };

    /** The colorspace to apply (if any). */
    UPROPERTY(EditAnywhere)
    ERuleRangerTextureColorSpace ColorSpace{ ERuleRangerTextureColorSpace::Auto };

    /** The valid TextureCompression settings. */
    UPROPERTY(EditAnywhere)
    TArray<TEnumAsByte<TextureCompressionSettings>> TextureCompressionSettings;

    /** The valid TextureGroups. */
    UPROPERTY(EditAnywhere)
    TArray<TEnumAsByte<TextureGroup>> TextureGroups;

    /** The valid MipGen settings. */
    UPROPERTY(EditAnywhere)
    TArray<TEnumAsByte<TextureMipGenSettings>> TextureMipGenSettings;

    /** The constraint to check. */
    UPROPERTY(EditAnywhere)
    ETextureResolutionConstraint TextureResolutionConstraint{ ETextureResolutionConstraint::PowerOfTwo };
};
