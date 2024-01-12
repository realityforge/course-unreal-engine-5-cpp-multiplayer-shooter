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
#include "K2Node_FunctionEntry.h"
#include "RuleRangerAction.h"
#include "EnsureVariablesHaveDescriptionsAction.generated.h"

/**
 * Action to check that the variable defined in the Blueprint have descriptions.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"))
class RULERANGER_API UEnsureVariablesHaveDescriptionsAction final : public UBaseAnalyzeVariableAction
{
    GENERATED_BODY()

    /** Should the action check instance editable variables. */
    UPROPERTY(EditAnywhere, Category = "Rule Ranger")
    bool bCheckInstanceEditableVariables{ true };

    /** Should the action check instance editable variables. */
    UPROPERTY(EditAnywhere, Category = "Rule Ranger")
    bool bCheckTransientVariables{ false };

    /** Should the action check instance editable variables. */
    UPROPERTY(EditAnywhere, Category = "Rule Ranger")
    bool bCheckPrivateVariables{ false };

    /** Should the action check local variables in functions. */
    UPROPERTY(EditAnywhere, Category = "Rule Ranger")
    bool bCheckLocalVariables{ true };

protected:
    virtual bool ShouldAnalyzeGraph(UEdGraph* Graph) const override;

    virtual void AnalyzeVariable(URuleRangerActionContext* ActionContext,
                                 UBlueprint* Blueprint,
                                 const FBPVariableDescription& Variable,
                                 UEdGraph* Graph) override;
};
