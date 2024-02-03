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
#include "EnsureFunctionsMaxNodeCountAction.generated.h"

/**
 * Action to ensure that a function has a maximum number of "non-trivial" nodes.
 */
UCLASS()
class RULERANGER_API UEnsureFunctionsMaxNodeCountAction final : public UBlueprintFunctionActionBase
{
    GENERATED_BODY()

    /** Should the action check protected functions. */
    UPROPERTY(EditAnywhere)
    int32 MaxNonTrivialNodeCount{ 50 };

    static bool IsNodeTrivial(const UEdGraphNode* Node);

protected:
    virtual void AnalyzeFunction(URuleRangerActionContext* ActionContext,
                                 UBlueprint* Blueprint,
                                 UK2Node_FunctionEntry* FunctionEntry,
                                 UEdGraph* Graph) override;
};
