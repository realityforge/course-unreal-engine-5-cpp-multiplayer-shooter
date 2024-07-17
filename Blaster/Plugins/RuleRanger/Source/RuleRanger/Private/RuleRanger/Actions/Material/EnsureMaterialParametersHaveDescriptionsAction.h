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
#include "EnsureMaterialParametersHaveDescriptionsAction.generated.h"

/**
 * Action to check that the parameters defined in the Material have descriptions.
 */
UCLASS(DisplayName = "Ensure Material Parameters Have Descriptions")
class RULERANGER_API UEnsureMaterialParametersHaveDescriptionsAction final : public URuleRangerAction
{
    GENERATED_BODY()

    void AnalyzeParameters(URuleRangerActionContext* ActionContext,
                           UObject* Object,
                           const UMaterial* const Material,
                           EMaterialParameterType Type,
                           TMap<FMaterialParameterInfo, FMaterialParameterMetadata> Parameters) const;

protected:
    void AnalyzeParameter(URuleRangerActionContext* ActionContext,
                          UObject* Object,
                          const UMaterial* Material,
                          EMaterialParameterType Type,
                          const FMaterialParameterInfo& MaterialParameterInfo,
                          const FMaterialParameterMetadata& Metadata) const;

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() override;
};
