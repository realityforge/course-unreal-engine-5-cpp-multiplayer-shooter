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

void UEnsureMaterialParametersHaveDescriptionsAction::AnalyzeParameter(
    URuleRangerActionContext* ActionContext,
    [[maybe_unused]] UObject* Object,
    const UMaterial* const Material,
    [[maybe_unused]] EMaterialParameterType Type,
    const FMaterialParameterInfo& MaterialParameterInfo,
    const FMaterialParameterMetadata& Metadata) const
{
    // TODO: Extract out superclass that can be used to check parameter names

    if (Metadata.Description.TrimStartAndEnd().IsEmpty())
    {
        ActionContext->Error(
            FText::FromString(FString::Printf(TEXT("Material contains a parameter named '%s' that is expected "
                                                   "to have a description but does not."),
                                              *MaterialParameterInfo.Name.ToString())));
    }
    else
    {
        LogInfo(Material,
                FString::Printf(TEXT("Parameter named '%s' has a description as expected."),
                                *MaterialParameterInfo.Name.ToString()));
    }
}

void UEnsureMaterialParametersHaveDescriptionsAction::AnalyzeParameters(
    URuleRangerActionContext* ActionContext,
    UObject* Object,
    const UMaterial* const Material,
    const EMaterialParameterType Type,
    const TMap<FMaterialParameterInfo, FMaterialParameterMetadata> Parameters) const
{
    for (auto ParameterIt = Parameters.CreateConstIterator(); ParameterIt; ++ParameterIt)
    {
        AnalyzeParameter(ActionContext, Object, Material, Type, ParameterIt->Key, ParameterIt->Value);
    }
}

void UEnsureMaterialParametersHaveDescriptionsAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                           UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto Material = CastChecked<UMaterial>(Object);

    LogInfo(Material, FString::Printf(TEXT("Processing Material named '%s'."), *Material->GetName()));

    TMap<FMaterialParameterInfo, FMaterialParameterMetadata> Parameters;

    Material->GetAllParametersOfType(EMaterialParameterType::Scalar, Parameters);
    AnalyzeParameters(ActionContext, Object, Material, EMaterialParameterType::Scalar, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::Vector, Parameters);
    AnalyzeParameters(ActionContext, Object, Material, EMaterialParameterType::Vector, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::DoubleVector, Parameters);
    AnalyzeParameters(ActionContext, Object, Material, EMaterialParameterType::DoubleVector, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::Texture, Parameters);
    AnalyzeParameters(ActionContext, Object, Material, EMaterialParameterType::Texture, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::Font, Parameters);
    AnalyzeParameters(ActionContext, Object, Material, EMaterialParameterType::Font, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::RuntimeVirtualTexture, Parameters);
    AnalyzeParameters(ActionContext, Object, Material, EMaterialParameterType::RuntimeVirtualTexture, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::SparseVolumeTexture, Parameters);
    AnalyzeParameters(ActionContext, Object, Material, EMaterialParameterType::SparseVolumeTexture, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::StaticSwitch, Parameters);
    AnalyzeParameters(ActionContext, Object, Material, EMaterialParameterType::StaticSwitch, Parameters);

    Parameters.Reset();

    Material->GetAllParametersOfType(EMaterialParameterType::StaticComponentMask, Parameters);
    AnalyzeParameters(ActionContext, Object, Material, EMaterialParameterType::StaticComponentMask, Parameters);
}

UClass* UEnsureMaterialParametersHaveDescriptionsAction::GetExpectedType()
{
    return UMaterial::StaticClass();
}
