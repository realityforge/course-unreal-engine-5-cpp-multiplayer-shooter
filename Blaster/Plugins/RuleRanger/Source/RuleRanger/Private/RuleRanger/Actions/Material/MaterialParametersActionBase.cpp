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
#include "MaterialParametersActionBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MaterialParametersActionBase)

void UMaterialParametersActionBase::AnalyzeParameter(URuleRangerActionContext* ActionContext,
                                                     const UMaterial* const Material,
                                                     const FMaterialParameterInfo& Info,
                                                     const FMaterialParameterMetadata& Metadata) const
{
    LogError(Material, TEXT("Action failed to override AnalyzeFunction."));
}

void UMaterialParametersActionBase::AnalyzeParameters(
    URuleRangerActionContext* ActionContext,
    const UMaterial* const Material,
    const TMap<FMaterialParameterInfo, FMaterialParameterMetadata>& Parameters) const
{
    if (ShouldAnalyzeParameters(ActionContext, Material, Parameters))
    {
        for (auto ParameterIt = Parameters.CreateConstIterator(); ParameterIt; ++ParameterIt)
        {
            AnalyzeParameter(ActionContext, Material, ParameterIt->Key, ParameterIt->Value);
        }
    }
}

bool UMaterialParametersActionBase::ShouldAnalyzeParameters(
    URuleRangerActionContext* ActionContext,
    const UMaterial* const Material,
    const TMap<FMaterialParameterInfo, FMaterialParameterMetadata>& Parameters) const
{
    return true;
}

void UMaterialParametersActionBase::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto Material = CastChecked<UMaterial>(Object);

    LogInfo(Material, FString::Printf(TEXT("Processing Material named '%s'."), *Material->GetName()));

    TMap<FMaterialParameterInfo, FMaterialParameterMetadata> AllParameters;
    TMap<FMaterialParameterInfo, FMaterialParameterMetadata> Parameters;

    Material->GetAllParametersOfType(EMaterialParameterType::Scalar, Parameters);
    AllParameters.Append(Parameters);

    Material->GetAllParametersOfType(EMaterialParameterType::Vector, Parameters);
    AllParameters.Append(Parameters);

    Material->GetAllParametersOfType(EMaterialParameterType::DoubleVector, Parameters);
    AllParameters.Append(Parameters);

    Material->GetAllParametersOfType(EMaterialParameterType::Texture, Parameters);
    AllParameters.Append(Parameters);

    Material->GetAllParametersOfType(EMaterialParameterType::Font, Parameters);
    AllParameters.Append(Parameters);

    Material->GetAllParametersOfType(EMaterialParameterType::RuntimeVirtualTexture, Parameters);
    AllParameters.Append(Parameters);

    Material->GetAllParametersOfType(EMaterialParameterType::SparseVolumeTexture, Parameters);
    AllParameters.Append(Parameters);

    Material->GetAllParametersOfType(EMaterialParameterType::StaticSwitch, Parameters);
    AllParameters.Append(Parameters);

    Material->GetAllParametersOfType(EMaterialParameterType::StaticComponentMask, Parameters);
    AllParameters.Append(Parameters);

    AnalyzeParameters(ActionContext, Material, AllParameters);
}

UClass* UMaterialParametersActionBase::GetExpectedType()
{
    return UMaterial::StaticClass();
}
