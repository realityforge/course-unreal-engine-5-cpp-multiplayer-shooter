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
#include "EnsureVariablesHaveDescriptionsAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureVariablesHaveDescriptionsAction)

bool UEnsureVariablesHaveDescriptionsAction::ShouldAnalyzeFunction(UEdGraph* Graph,
                                                                   UK2Node_FunctionEntry* FunctionEntry) const
{
    // Only analyze variables in graphs if we should check local variables
    return bCheckLocalVariables && Super::ShouldAnalyzeFunction(Graph, FunctionEntry);
}

void UEnsureVariablesHaveDescriptionsAction::AnalyzeVariable(URuleRangerActionContext* ActionContext,
                                                             UBlueprint* Blueprint,
                                                             const FBPVariableDescription& Variable,
                                                             UK2Node_FunctionEntry* FunctionEntry,
                                                             UEdGraph* Graph)
{
    const bool bDisableEditOnInstance =
        CPF_DisableEditOnInstance == (CPF_DisableEditOnInstance & Variable.PropertyFlags);
    const bool bTransient = CPF_Transient == (CPF_Transient & Variable.PropertyFlags);
    const auto Property = FindUFieldOrFProperty<FProperty>(Blueprint->GeneratedClass, Variable.VarName);
    // ReSharper disable once CppTooWideScopeInitStatement
    const bool bPrivate = Property && Property->GetBoolMetaData(FName("BlueprintPrivate"));

    if (!bCheckLocalVariables && Graph)
    {
        LogInfo(Blueprint,
                FString::Printf(TEXT("Skipping check of local variable named "
                                     "'%s' in function named '%s' as "
                                     "bCheckLocalVariables is set to false."),
                                *Variable.VarName.ToString(),
                                *Graph->GetName()));
    }
    else if (!bCheckInstanceEditableVariables && !bDisableEditOnInstance)
    {
        LogInfo(Blueprint,
                FString::Printf(TEXT("Skipping check of variable named "
                                     "'%s' as bCheckInstanceEditableVariables is set to "
                                     "false and the variable is an instance editable variable."),
                                *Variable.VarName.ToString()));
    }
    else if (!bCheckNonInstanceEditableVariables && bDisableEditOnInstance)
    {
        LogInfo(Blueprint,
                FString::Printf(TEXT("Skipping check of variable named "
                                     "'%s' as bCheckNonInstanceEditableVariables is set to "
                                     "false and the variable is not an instance editable variable."),
                                *Variable.VarName.ToString()));
    }
    else if (!Graph && !bCheckTransientVariables && bTransient)
    {
        LogInfo(Blueprint,
                FString::Printf(TEXT("Skipping check of variable named "
                                     "'%s' as bCheckTransientVariables is set to "
                                     "false and the variable is a transient variable."),
                                *Variable.VarName.ToString()));
    }
    else if (!Graph && !bCheckPrivateVariables && bPrivate)
    {
        LogInfo(Blueprint,
                FString::Printf(TEXT("Skipping check of variable named "
                                     "'%s' as bCheckPrivateVariables is set to "
                                     "false and the variable is a private variable."),
                                *Variable.VarName.ToString()));
    }
    else
    {
        if (!Variable.HasMetaData(FBlueprintMetadata::MD_Tooltip)
            || 0 == Variable.GetMetaData(FBlueprintMetadata::MD_Tooltip).Len())
        {
            const FString Name = Variable.VarName.ToString();
            const FText TypeName = UEdGraphSchema_K2::TypeToText(Variable.VarType);
            const auto& Hint1 = !bCheckPrivateVariables && !bPrivate
                ? TEXT(" As bCheckPrivateVariables is false, this warning can be "
                       "suppressed by marking the variable as private.")
                : TEXT("");
            const auto& Hint2 = !bCheckTransientVariables && !bTransient
                ? TEXT(" As bCheckTransientVariables is false, this warning can be "
                       "suppressed by marking the variable as transient.")
                : TEXT("");
            const auto& Hint3 = !bCheckNonInstanceEditableVariables && !bDisableEditOnInstance
                ? TEXT(" As bCheckNonInstanceEditableVariables is false, this warning can be "
                       "suppressed by disabling the Instance Editable setting.")
                : TEXT("");
            const auto& Hint4 = !bCheckInstanceEditableVariables && bDisableEditOnInstance
                ? TEXT(" As bCheckInstanceEditableVariables is false, this warning can be "
                       "suppressed by enabling the Instance Editable setting.")
                : TEXT("");

            if (Graph)
            {
                const auto& ErrorMessage = FString::Printf(TEXT("Blueprint contains a local variable "
                                                                "named '%s' in the function "
                                                                "named '%s' that is expected "
                                                                "to have a description but does not.%s%s%s%s"),
                                                           *Name,
                                                           *Graph->GetName(),
                                                           Hint1,
                                                           Hint2,
                                                           Hint3,
                                                           Hint4);
                ActionContext->Error(FText::FromString(ErrorMessage));
            }
            else
            {
                const auto& ErrorMessage = FString::Printf(TEXT("Blueprint contains a variable "
                                                                "named '%s' that is expected "
                                                                "to have a description but does not.%s%s%s%s"),
                                                           *Name,
                                                           Hint1,
                                                           Hint2,
                                                           Hint3,
                                                           Hint4);
                ActionContext->Error(FText::FromString(ErrorMessage));
            }
        }
        else
        {
            if (Graph)
            {
                LogInfo(Blueprint,
                        FString::Printf(TEXT("Local variable "
                                             "named '%s' in the function "
                                             "named '%s' has a description as expected."),
                                        *Variable.VarName.ToString(),
                                        *Graph->GetName()));
            }
            else
            {
                LogInfo(Blueprint,
                        FString::Printf(TEXT("Variable named '%s' has a description as expected."),
                                        *Variable.VarName.ToString()));
            }
        }
    }
}
