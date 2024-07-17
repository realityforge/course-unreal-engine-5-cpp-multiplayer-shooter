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

void UMaterialParametersActionBase::AnalyzeParameter(URuleRangerActionContext* ActionContext,
                                                     const UMaterial* const Material,
                                                     EMaterialParameterType Type,
                                                     const FMaterialParameterInfo& Info,
                                                     const FMaterialParameterMetadata& Metadata) const
{
    LogError(Material, TEXT("Action failed to override AnalyzeFunction."));
}

void UMaterialParametersActionBase::AnalyzeParameters(
    URuleRangerActionContext* ActionContext,
    const UMaterial* const Material,
    const EMaterialParameterType MaterialParameterType,
    const TMap<FMaterialParameterInfo, FMaterialParameterMetadata>& Parameters) const
{
    for (auto ParameterIt = Parameters.CreateConstIterator(); ParameterIt; ++ParameterIt)
    {
        AnalyzeParameter(ActionContext, Material, MaterialParameterType, ParameterIt->Key, ParameterIt->Value);
    }
}

void UMaterialParametersActionBase::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto Material = CastChecked<UMaterial>(Object);

    LogInfo(Material, FString::Printf(TEXT("Processing Material named '%s'."), *Material->GetName()));

    TMap<FMaterialParameterInfo, FMaterialParameterMetadata> Parameters;

    Material->GetAllParametersOfType(EMaterialParameterType::Scalar, Parameters);
    AnalyzeParameters(ActionContext, Material, EMaterialParameterType::Scalar, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::Vector, Parameters);
    AnalyzeParameters(ActionContext, Material, EMaterialParameterType::Vector, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::DoubleVector, Parameters);
    AnalyzeParameters(ActionContext, Material, EMaterialParameterType::DoubleVector, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::Texture, Parameters);
    AnalyzeParameters(ActionContext, Material, EMaterialParameterType::Texture, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::Font, Parameters);
    AnalyzeParameters(ActionContext, Material, EMaterialParameterType::Font, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::RuntimeVirtualTexture, Parameters);
    AnalyzeParameters(ActionContext, Material, EMaterialParameterType::RuntimeVirtualTexture, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::SparseVolumeTexture, Parameters);
    AnalyzeParameters(ActionContext, Material, EMaterialParameterType::SparseVolumeTexture, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::StaticSwitch, Parameters);
    AnalyzeParameters(ActionContext, Material, EMaterialParameterType::StaticSwitch, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::StaticComponentMask, Parameters);
    AnalyzeParameters(ActionContext, Material, EMaterialParameterType::StaticComponentMask, Parameters);
}

UClass* UMaterialParametersActionBase::GetExpectedType()
{
    return UMaterial::StaticClass();
}
