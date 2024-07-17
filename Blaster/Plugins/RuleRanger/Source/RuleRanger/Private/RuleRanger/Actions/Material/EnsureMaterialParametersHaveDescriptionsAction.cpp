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
#include "Materials/MaterialExpressionParameter.h"


//
// bool UEnsureMaterialParametersHaveDescriptionsAction::ShouldAnalyzeFunction(UEdGraph* Graph,
//                                                                             UK2Node_FunctionEntry* FunctionEntry) const
// {
//     // Only analyze variables in graphs if we should check local variables
//     return bCheckLocalVariables && Super::ShouldAnalyzeFunction(Graph, FunctionEntry);
// }
//
// void UEnsureMaterialParametersHaveDescriptionsAction::AnalyzeVariable(URuleRangerActionContext* ActionContext,
//                                                                       UBlueprint* Blueprint,
//                                                                       const FBPVariableDescription& Variable,
//                                                                       UK2Node_FunctionEntry* FunctionEntry,
//                                                                       UEdGraph* Graph)
// {
//     bool bCheckVariable = true;
//     if (!bCheckLocalVariables && Graph)
//     {
//         LogInfo(Blueprint,
//                 FString::Printf(TEXT("Skipping check of local variable named "
//                                      "'%s' in function named '%s' as "
//                                      "bCheckLocalVariables is set to false."),
//                                 *Variable.VarName.ToString(),
//                                 *Graph->GetName()));
//         bCheckVariable = false;
//     }
//     else if (!bCheckInstanceEditableVariables
//              && CPF_DisableEditOnInstance != (CPF_DisableEditOnInstance & Variable.PropertyFlags))
//     {
//         LogInfo(Blueprint,
//                 FString::Printf(TEXT("Skipping check of variable named "
//                                      "'%s' as bCheckInstanceEditableVariables is set to "
//                                      "false and the variable is an instance editable variable."),
//                                 *Variable.VarName.ToString()));
//         bCheckVariable = false;
//     }
//
//     if (!Graph && bCheckVariable && !bCheckTransientVariables
//         && CPF_Transient == (CPF_Transient & Variable.PropertyFlags))
//     {
//         LogInfo(Blueprint,
//                 FString::Printf(TEXT("Skipping check of variable named "
//                                      "'%s' as bCheckTransientVariables is set to "
//                                      "false and the variable is a transient variable."),
//                                 *Variable.VarName.ToString()));
//         bCheckVariable &= false;
//     }
//
//     // ReSharper disable once CppTooWideScopeInitStatement
//     const auto Property = FindUFieldOrFProperty<FProperty>(Blueprint->GeneratedClass, Variable.VarName);
//     if (!Graph && bCheckVariable && !bCheckPrivateVariables
//         && (Property && Property->GetBoolMetaData(FName("BlueprintPrivate"))))
//     {
//         LogInfo(Blueprint,
//                 FString::Printf(TEXT("Skipping check of variable named "
//                                      "'%s' as bCheckPrivateVariables is set to "
//                                      "false and the variable is a private variable."),
//                                 *Variable.VarName.ToString()));
//         bCheckVariable &= false;
//     }
//
//     if (bCheckVariable)
//     {
//         if (!Variable.HasMetaData(FBlueprintMetadata::MD_Tooltip)
//             || 0 == Variable.GetMetaData(FBlueprintMetadata::MD_Tooltip).Len())
//         {
//             const FString Name = Variable.VarName.ToString();
//             const FText TypeName = UEdGraphSchema_K2::TypeToText(Variable.VarType);
//             if (Graph)
//             {
//                 const auto& ErrorMessage = FString::Printf(TEXT("Blueprint contains a local variable "
//                                                                 "named '%s' in the function "
//                                                                 "named '%s' that is expected "
//                                                                 "to have a description but does not."),
//                                                            *Name,
//                                                            *Graph->GetName());
//                 ActionContext->Error(FText::FromString(ErrorMessage));
//             }
//             else
//             {
//                 const auto& ErrorMessage = FString::Printf(TEXT("Blueprint contains a variable "
//                                                                 "named '%s' that is expected "
//                                                                 "to have a description but does not."),
//                                                            *Name);
//                 ActionContext->Error(FText::FromString(ErrorMessage));
//             }
//         }
//         else
//         {
//             if (Graph)
//             {
//                 LogInfo(Blueprint,
//                         FString::Printf(TEXT("Local variable "
//                                              "named '%s' in the function "
//                                              "named '%s' has a description as expected."),
//                                         *Variable.VarName.ToString(),
//                                         *Graph->GetName()));
//             }
//             else
//             {
//                 LogInfo(Blueprint,
//                         FString::Printf(TEXT("Variable named '%s' has a description as expected."),
//                                         *Variable.VarName.ToString()));
//             }
//         }
//     }
// }

void UEnsureMaterialParametersHaveDescriptionsAction::AnalyzeParameter(URuleRangerActionContext* ActionContext,
                                                                       UObject* Object,
                                                                       UMaterial* const Material,
                                                                       EMaterialParameterType Type,
                                                                       FMaterialParameterInfo MaterialParameterInfo,
                                                                       FMaterialParameterMetadata Metadata)
{
    //TODO: Extract out superclass that can be used to check parameter names
    //TODO: Code cleanup

    if (Metadata.Description.TrimStartAndEnd().IsEmpty())
    {
        ActionContext->Error(FText::FromString(FString::Printf(
            TEXT("Material contains a parameter named '%s' that is expected "
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

void UEnsureMaterialParametersHaveDescriptionsAction::AnalyzeParameters(URuleRangerActionContext* ActionContext,
                                                                        UObject* Object,
                                                                        UMaterial* const Material,
                                                                        const EMaterialParameterType Type,
                                                                        TMap<FMaterialParameterInfo,
                                                                             FMaterialParameterMetadata> Parameters)
{
    for (auto ParameterIt = Parameters.CreateIterator(); ParameterIt; ++ParameterIt)
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
