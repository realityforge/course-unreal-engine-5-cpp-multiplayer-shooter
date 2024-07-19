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
#include "MaterialParametersActionBase.h"
#include "RuleRangerAction.h"
#include "EnsureMaterialParameterNamesMatchRegexAction.generated.h"

/**
 * Action to check that the names of the parameters defined in the Material match a regex pattern.
 */
UCLASS(DisplayName = "Ensure Material Parameter Names Match Regex")
class RULERANGER_API UEnsureMaterialParameterNamesMatchRegexAction final : public UMaterialParametersActionBase
{
    GENERATED_BODY()

    /** The regex pattern that the parameter name is expected to match. */
    UPROPERTY(EditAnywhere)
    FString Pattern{ TEXT("^[A-Z][a-z0-9A-Z_]*$") };
    /** A flag controlling whether matching is Case Sensitive or not. */
    UPROPERTY(EditAnywhere)
    bool bCaseSensitive{ true };

protected:
    virtual void AnalyzeParameter(URuleRangerActionContext* ActionContext,
                                  const UMaterial* Material,
                                  const FMaterialParameterInfo& Info,
                                  const FMaterialParameterMetadata& Metadata) const override;
};
