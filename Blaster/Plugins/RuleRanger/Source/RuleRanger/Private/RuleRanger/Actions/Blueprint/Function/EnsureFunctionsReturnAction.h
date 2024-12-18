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

#include "BlueprintFunctionActionBase.h"
#include "CoreMinimal.h"
#include "RuleRangerAction.h"
#include "EnsureFunctionsReturnAction.generated.h"

/**
 * Action to check that the functions have a return node.
 * (We let the EnsureNoUnlinkedNodesAction verify that the result node is connected).
 */
UCLASS(DisplayName = "Ensure Blueprint Functions Return")
class RULERANGER_API UEnsureFunctionsReturnAction final : public UBlueprintFunctionActionBase
{
    GENERATED_BODY()

protected:
    virtual bool ShouldAnalyzeGraph(UBlueprint* Blueprint, UEdGraph* Graph) const override;

    virtual void AnalyzeFunction(URuleRangerActionContext* ActionContext,
                                 UBlueprint* Blueprint,
                                 UK2Node_FunctionEntry* FunctionEntry,
                                 UEdGraph* Graph) override;
};
