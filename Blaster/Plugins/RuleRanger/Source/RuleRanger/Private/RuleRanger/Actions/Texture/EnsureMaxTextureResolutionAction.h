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
#include "RuleRangerAction.h"
#include "Texture2DActionBase.h"
#include "EnsureMaxTextureResolutionAction.generated.h"

/**
 * Action to check that a Texture dimensions comply with the specified constraint.
 */
UCLASS(DisplayName = "Ensure Max Texture Resolution")
class RULERANGER_API UEnsureMaxTextureResolutionAction final : public UTexture2DActionBase
{
    GENERATED_BODY()

    /**
     * The maximum resolution of the texture in X dimension.
     * If set to 0 then there is no maximum resolution.
     */
    UPROPERTY(EditAnywhere)
    int32 MaxSizeX{ 4096 };

    /**
     * The maximum resolution of the texture in Y dimension.
     * If set to 0 then there is no maximum resolution.
     */
    UPROPERTY(EditAnywhere)
    int32 MaxSizeY{ 4096 };

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;
};
