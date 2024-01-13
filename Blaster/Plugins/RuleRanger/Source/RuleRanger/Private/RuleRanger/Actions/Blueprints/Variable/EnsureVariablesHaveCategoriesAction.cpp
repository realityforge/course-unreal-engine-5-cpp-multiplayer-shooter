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

#include "EnsureVariablesHaveCategoriesAction.h"
#include "K2Node_FunctionEntry.h"
#include "Kismet2/BlueprintEditorUtils.h"

bool UEnsureVariablesHaveCategoriesAction::ShouldAnalyzeBlueprintVariables(UBlueprint* Blueprint) const
{
    // Only perform more complicated checks if the number of variables could exceed threshold
    if (Blueprint->NewVariables.Num() >= Threshold)
    {
        int32 VariableCount = Blueprint->NewVariables.Num();
        for (const auto& Variable : Blueprint->NewVariables)
        {
            // Ignore components
            if (FBlueprintEditorUtils::IsVariableComponent(Variable))
            {
                VariableCount--;
            }
            if (!bCheckInstanceEditableVariables
                && CPF_DisableEditOnInstance != (CPF_DisableEditOnInstance & Variable.PropertyFlags))
            {
                VariableCount--;
            }
        }
        return VariableCount >= Threshold;
    }
    return false;
}

bool UEnsureVariablesHaveCategoriesAction::ShouldAnalyzeFunction(UEdGraph* Graph,
                                                                 UK2Node_FunctionEntry* FunctionEntry) const
{
    // Only analyze variables in graphs if we should check local variables
    return bCheckLocalVariables && Super::ShouldAnalyzeFunction(Graph, FunctionEntry);
}

void UEnsureVariablesHaveCategoriesAction::AnalyzeVariable(URuleRangerActionContext* ActionContext,
                                                           UBlueprint* Blueprint,
                                                           const FBPVariableDescription& Variable,
                                                           UK2Node_FunctionEntry* FunctionEntry,
                                                           UEdGraph* Graph)
{
    if (FunctionEntry)
    {
        // Processing LocalVariable
        if (FunctionEntry->LocalVariables.Num() >= Threshold)
        {
            if (Variable.Category.ToString().Equals(TEXT("Default")))
            {
                const auto& ErrorMessage = FString::Printf(TEXT("Blueprint contains a local variable named "
                                                                "'%s' in function named '%s' that is in the Default "
                                                                "category but it is expected to have a non-default "
                                                                "category specified as the function has "
                                                                "more than %d variables."),
                                                           *Variable.VarName.ToString(),
                                                           *Graph->GetName(),
                                                           Threshold);
                ActionContext->Error(FText::FromString(ErrorMessage));
            }
        }
    }
    else
    {
        // Processing Blueprint Variable
        if (Blueprint->NewVariables.Num() >= Threshold)
        {
            int32 VariableCount = Blueprint->NewVariables.Num();
            for (FBPVariableDescription Desc : Blueprint->NewVariables)
            {
                // Ignore components
                if (FBlueprintEditorUtils::IsVariableComponent(Desc))
                {
                    VariableCount--;
                }
            }
            if (VariableCount >= Threshold)
            {
                if (Variable.Category.ToString().Equals(TEXT("Default")))
                {
                    const auto& ErrorMessage = FString::Printf(
                        TEXT("Blueprint contains a variable named "
                             "'%s' that is in the Default category but it is expected to have a non-default "
                             "category specified as the Blueprint has "
                             "more than %d variables."),
                        *Variable.VarName.ToString(),
                        Threshold);
                    ActionContext->Error(FText::FromString(ErrorMessage));
                }
            }
        }
    }
}
