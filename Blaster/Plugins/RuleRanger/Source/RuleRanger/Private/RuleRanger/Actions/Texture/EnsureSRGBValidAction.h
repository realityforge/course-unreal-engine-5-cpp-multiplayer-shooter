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
#include "EnsureSRGBValidAction.generated.h"

/**
 * Action to check that a Texture has a valid sRGB value.
 */
UCLASS(DisplayName = "Ensure Texture sRGB is Valid")
class RULERANGER_API UEnsureSRGBValidAction final : public UTexture2DActionBase
{
    GENERATED_BODY()

    /** The valid sRGB settings. */
    UPROPERTY(EditAnywhere)
    bool bSRGB{ false };

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;
};
