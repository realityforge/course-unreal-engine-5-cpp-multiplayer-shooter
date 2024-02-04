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

#include "BlueprintVariableActionBase.h"
#include "CoreMinimal.h"
#include "RuleRangerAction.h"
#include "EnsureVariablesHaveDescriptionsAction.generated.h"

/**
 * Action to check that the variable defined in the Blueprint have descriptions.
 */
UCLASS(DisplayName = "Ensure Blueprint Variables Have Descriptions")
class RULERANGER_API UEnsureVariablesHaveDescriptionsAction final : public UBlueprintVariableActionBase
{
    GENERATED_BODY()

    /** Should the action check instance editable variables. */
    UPROPERTY(EditAnywhere)
    bool bCheckInstanceEditableVariables{ true };

    /** Should the action check instance editable variables. */
    UPROPERTY(EditAnywhere)
    bool bCheckTransientVariables{ false };

    /** Should the action check private variables. */
    UPROPERTY(EditAnywhere)
    bool bCheckPrivateVariables{ false };

    /** Should the action check local variables in functions. */
    UPROPERTY(EditAnywhere)
    bool bCheckLocalVariables{ true };

protected:
    virtual bool ShouldAnalyzeFunction(UEdGraph* Graph, UK2Node_FunctionEntry* FunctionEntry) const override;

    virtual void AnalyzeVariable(URuleRangerActionContext* ActionContext,
                                 UBlueprint* Blueprint,
                                 const FBPVariableDescription& Variable,
                                 UK2Node_FunctionEntry* FunctionEntry,
                                 UEdGraph* Graph) override;
};
