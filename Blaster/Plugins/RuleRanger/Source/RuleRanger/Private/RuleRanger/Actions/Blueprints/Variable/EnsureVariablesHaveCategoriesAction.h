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

#include "BaseAnalyzeVariableAction.h"
#include "CoreMinimal.h"
#include "RuleRangerAction.h"
#include "EnsureVariablesHaveCategoriesAction.generated.h"

/**
 * Action to check that the variables defined in the Blueprint have a category if the Blueprint crosses a threshold
 * number of variables.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"))
class RULERANGER_API UEnsureVariablesHaveCategoriesAction final : public UBaseAnalyzeVariableAction
{
    GENERATED_BODY()

    /** The number of functions that triggers requirement that all variables have a category. */
    UPROPERTY(EditAnywhere, Category = "Rule Ranger")
    int32 Threshold{ 5 };

    /** Should the action check instance editable variables. */
    UPROPERTY(EditAnywhere, Category = "Rule Ranger")
    bool bCheckInstanceEditableVariables{ true };

    /** Should the action check local variables in functions. */
    UPROPERTY(EditAnywhere, Category = "Rule Ranger")
    bool bCheckLocalVariables{ true };

protected:
    virtual bool ShouldAnalyzeBlueprintVariables(UBlueprint* Blueprint) const override;

    virtual bool ShouldAnalyzeFunction(UEdGraph* Graph, UK2Node_FunctionEntry* FunctionEntry) const override;

    virtual void AnalyzeVariable(URuleRangerActionContext* ActionContext,
                                 UBlueprint* Blueprint,
                                 const FBPVariableDescription& Variable,
                                 UK2Node_FunctionEntry* FunctionEntry,
                                 UEdGraph* Graph) override;
};
