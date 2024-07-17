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

#include "EnsureMaterialParametersHaveDescriptionsAction.h"

void UEnsureMaterialParametersHaveDescriptionsAction::AnalyzeParameter(URuleRangerActionContext* ActionContext,
                                                                       const UMaterial* const Material,
                                                                       [[maybe_unused]] EMaterialParameterType Type,
                                                                       const FMaterialParameterInfo& Info,
                                                                       const FMaterialParameterMetadata& Metadata) const
{
    if (Metadata.Description.TrimStartAndEnd().IsEmpty())
    {
        ActionContext->Error(
            FText::FromString(FString::Printf(TEXT("Material contains a parameter named '%s' that is expected "
                                                   "to have a description but does not."),
                                              *Info.Name.ToString())));
    }
    else
    {
        LogInfo(Material,
                FString::Printf(TEXT("Parameter named '%s' has a description as expected."), *Info.Name.ToString()));
    }
}
