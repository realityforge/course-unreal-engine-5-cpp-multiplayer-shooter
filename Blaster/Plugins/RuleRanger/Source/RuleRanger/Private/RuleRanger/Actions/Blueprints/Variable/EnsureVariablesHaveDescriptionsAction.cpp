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
    bool bCheckVariable = true;
    if (!bCheckLocalVariables && Graph)
    {
        LogInfo(Blueprint,
                FString::Printf(TEXT("Skipping check of local variable named "
                                     "'%s' in function named '%s' as "
                                     "bCheckLocalVariables is set to false."),
                                *Variable.VarName.ToString(),
                                *Graph->GetName()));
        bCheckVariable = false;
    }
    else if (!bCheckInstanceEditableVariables
             && CPF_DisableEditOnInstance != (CPF_DisableEditOnInstance & Variable.PropertyFlags))
    {
        LogInfo(Blueprint,
                FString::Printf(TEXT("Skipping check of variable named "
                                     "'%s' as bCheckInstanceEditableVariables is set to "
                                     "false and the variable is an instance editable variable."),
                                *Variable.VarName.ToString()));
        bCheckVariable = false;
    }

    if (!Graph && bCheckVariable && !bCheckTransientVariables
        && CPF_Transient == (CPF_Transient & Variable.PropertyFlags))
    {
        LogInfo(Blueprint,
                FString::Printf(TEXT("Skipping check of variable named "
                                     "'%s' as bCheckTransientVariables is set to "
                                     "false and the variable is a transient variable."),
                                *Variable.VarName.ToString()));
        bCheckVariable &= false;
    }

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto Property = FindUFieldOrFProperty<FProperty>(Blueprint->GeneratedClass, Variable.VarName);
    if (!Graph && bCheckVariable && !bCheckPrivateVariables
        && (Property && Property->GetBoolMetaData(FName("BlueprintPrivate"))))
    {
        LogInfo(Blueprint,
                FString::Printf(TEXT("Skipping check of variable named "
                                     "'%s' as bCheckPrivateVariables is set to "
                                     "false and the variable is a private variable."),
                                *Variable.VarName.ToString()));
        bCheckVariable &= false;
    }

    if (bCheckVariable)
    {
        if (!Variable.HasMetaData(FBlueprintMetadata::MD_Tooltip)
            || 0 == Variable.GetMetaData(FBlueprintMetadata::MD_Tooltip).Len())
        {
            const FString Name = Variable.VarName.ToString();
            const FText TypeName = UEdGraphSchema_K2::TypeToText(Variable.VarType);
            if (Graph)
            {
                const auto& ErrorMessage = FString::Printf(TEXT("Blueprint contains a local variable "
                                                                "named '%s' in the function "
                                                                "named '%s' that is expected "
                                                                "to have a description but does not."),
                                                           *Name,
                                                           *Graph->GetName());
                ActionContext->Error(FText::FromString(ErrorMessage));
            }
            else
            {
                const auto& ErrorMessage = FString::Printf(TEXT("Blueprint contains a variable "
                                                                "named '%s' that is expected "
                                                                "to have a description but does not."),
                                                           *Name);
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
