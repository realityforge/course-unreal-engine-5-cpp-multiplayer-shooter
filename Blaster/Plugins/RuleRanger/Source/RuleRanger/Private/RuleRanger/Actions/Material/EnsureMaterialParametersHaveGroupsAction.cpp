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
#include "EnsureMaterialParametersHaveGroupsAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureMaterialParametersHaveGroupsAction)

void UEnsureMaterialParametersHaveGroupsAction::AnalyzeParameter(URuleRangerActionContext* ActionContext,
                                                                 const UMaterial* Material,
                                                                 const FMaterialParameterInfo& Info,
                                                                 const FMaterialParameterMetadata& Metadata) const
{
    if (Metadata.Group == EName::None)
    {
        ActionContext->Error(
            FText::FromString(FString::Printf(TEXT("Material contains a parameter named '%s' that is expected "
                                                   "to have a group but does not."),
                                              *Info.Name.ToString())));
    }
    else
    {
        LogInfo(Material,
                FString::Printf(TEXT("Parameter named '%s' has a group as expected."), *Info.Name.ToString()));
    }
}

bool UEnsureMaterialParametersHaveGroupsAction::ShouldAnalyzeParameters(
    URuleRangerActionContext* ActionContext,
    const UMaterial* const Material,
    const TMap<FMaterialParameterInfo, FMaterialParameterMetadata>& Parameters) const
{
    const int32 ParameterCount = Parameters.Num();
    const bool bAnalyze = ParameterCount >= Threshold;
    if (!bAnalyze)
    {
        LogInfo(Material,
                FString::Printf(TEXT("The number of parameters in the Material (%d)"
                                     " is below the threshold (%d) so it is not "
                                     "necessary to enforce grouping of the parameters."),
                                ParameterCount,
                                Threshold));
    }
    return bAnalyze;
}
