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
#include "EnsureTextureResolutionConstraintsAction.generated.h"

/**
 * The rules around texture resolution.
 */
UENUM(BlueprintType)
enum class ETextureResolutionConstraint : uint8
{
    /**
     * Both texture dimensions must be powers of two.
     * This is to ensure that the texture can have useful mipmap generation.
     */
    PowerOfTwo UMETA(DisplayName = "Power Of Two"),
    /**
     * Both texture dimensions must be divisible by 4.
     * Most texture compression formats require that the texture be divisible by 4 to use the texture compression
     * format.
     */
    DivisibleByFour UMETA(DisplayString = "Divisible by 4"),
    /**
     * Both texture dimensions must be divisible by 8.
     * Some texture compression formats require that the texture be divisible by 8 to use the texture compression
     * format.
     */
    DivisibleByEight UMETA(DisplayString = "Divisible by 8"),
    /**
     * Both texture dimensions must be divisible by 12.
     * Some texture compression formats require that the texture be divisible by 8 to use the texture compression
     * format.
     */
    DivisibleByTwelve UMETA(DisplayString = "Divisible by 12")
};

/**
 * Action to check that a Texture dimensions comply with the specified constraint.
 */
UCLASS()
class RULERANGER_API UEnsureTextureResolutionConstraintsAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** The constraint to check. */
    UPROPERTY(EditAnywhere)
    ETextureResolutionConstraint Constraint{ ETextureResolutionConstraint::PowerOfTwo };

    void CheckPowerOfTwo(URuleRangerActionContext* ActionContext, const UTexture2D* Texture) const;
    void CheckDivisibleConstraint(URuleRangerActionContext* ActionContext, const UTexture2D* Texture) const;

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() override;
};
