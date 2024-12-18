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

#include "BlueprintFunctionAccessFilteringActionBase.h"
#include "CoreMinimal.h"
#include "K2Node_FunctionEntry.h"
#include "RuleRangerAction.h"
#include "EnsureFunctionsHaveDescriptionsAction.generated.h"

/**
 * Action to check that the functions defined in the Blueprint have descriptions.
 */
UCLASS(DisplayName = "Ensure Blueprint Functions Have Descriptions")
class RULERANGER_API UEnsureFunctionsHaveDescriptionsAction final : public UBlueprintFunctionAccessFilteringActionBase
{
    GENERATED_BODY()

protected:
    virtual void AnalyzeFunction(URuleRangerActionContext* ActionContext,
                                 UBlueprint* Blueprint,
                                 UK2Node_FunctionEntry* FunctionEntry,
                                 UEdGraph* Graph) override;
};
