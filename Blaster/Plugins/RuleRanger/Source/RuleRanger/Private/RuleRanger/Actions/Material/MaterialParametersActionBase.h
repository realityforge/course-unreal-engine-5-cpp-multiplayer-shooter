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
#include "MaterialParametersActionBase.generated.h"

/**
 * Base actions for analyzing Material Parameters.
 */
UCLASS(Abstract)
class RULERANGER_API UMaterialParametersActionBase : public URuleRangerAction
{
    GENERATED_BODY()

    void AnalyzeParameters(URuleRangerActionContext* ActionContext,
                           const UMaterial* const Material,
                           const TMap<FMaterialParameterInfo, FMaterialParameterMetadata>& Parameters) const;

protected:
    virtual bool
    ShouldAnalyzeParameters(URuleRangerActionContext* ActionContext,
                            const UMaterial* const Material,
                            const TMap<FMaterialParameterInfo, FMaterialParameterMetadata>& Parameters) const;

    virtual void AnalyzeParameter(URuleRangerActionContext* ActionContext,
                                  const UMaterial* Material,
                                  const FMaterialParameterInfo& Info,
                                  const FMaterialParameterMetadata& Metadata) const;

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() override;
};
