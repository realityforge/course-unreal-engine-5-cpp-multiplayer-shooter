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
#include "EnsureMaterialParametersHaveDescriptionsAction.generated.h"

/**
 * Action to check that the parameters defined in the Material have descriptions.
 */
UCLASS(DisplayName = "Ensure Material Parameters Have Descriptions")
class RULERANGER_API UEnsureMaterialParametersHaveDescriptionsAction final : public UMaterialParametersActionBase
{
    GENERATED_BODY()

    /** The number of parameters that triggers the requirement that all parameters have a group. */
    UPROPERTY(EditAnywhere)
    int32 Threshold{ 5 };

protected:
    virtual void AnalyzeParameter(URuleRangerActionContext* ActionContext,
                                  const UMaterial* Material,
                                  const FMaterialParameterInfo& Info,
                                  const FMaterialParameterMetadata& Metadata) const override;
    virtual bool
    ShouldAnalyzeParameters(URuleRangerActionContext* ActionContext,
                            const UMaterial* const Material,
                            const TMap<FMaterialParameterInfo, FMaterialParameterMetadata>& Parameters) const override;
};
