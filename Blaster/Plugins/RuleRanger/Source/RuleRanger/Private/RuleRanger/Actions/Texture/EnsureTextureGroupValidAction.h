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
#include "Texture2DActionBase.h"
#include "EnsureTextureGroupValidAction.generated.h"

/**
 * Action to check that a Texture has a valid TextureGroup.
 */
UCLASS(DisplayName = "Ensure Texture Group Setting Is Valid")
class RULERANGER_API UEnsureTextureGroupValidAction final : public UTexture2DActionBase
{
    GENERATED_BODY()

    /** The valid TextureGroups. */
    UPROPERTY(EditAnywhere)
    TArray<TEnumAsByte<TextureGroup>> TextureGroups;

    /**
     * Flag to indicate that the first group should be used when fixing the asset.
     * If not set (or not groups specified) then no fix will be applied.
     */
    UPROPERTY(EditAnywhere)
    bool bApplyFix;

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;
};
