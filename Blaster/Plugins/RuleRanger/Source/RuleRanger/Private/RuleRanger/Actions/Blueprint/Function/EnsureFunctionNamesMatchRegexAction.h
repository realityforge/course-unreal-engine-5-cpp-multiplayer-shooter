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
#include "EnsureFunctionNamesMatchRegexAction.generated.h"

/**
 * Action to ensure that the names of functions match specified regex.
 */
UCLASS(DisplayName = "Ensure Blueprint Function Names Match Regex")
class RULERANGER_API UEnsureFunctionNamesMatchRegexAction final : public UBlueprintFunctionActionBase
{
    GENERATED_BODY()

    /** The regex pattern that the function name is expected to match. */
    UPROPERTY(EditAnywhere)
    FString Pattern{ TEXT("^[A-Z][a-z0-9A-Z_]*$") };
    /** A flag controlling whether matching is Case Sensitive or not. */
    UPROPERTY(EditAnywhere)
    bool bCaseSensitive{ true };

protected:
    virtual void AnalyzeFunction(URuleRangerActionContext* ActionContext,
                                 UBlueprint* Blueprint,
                                 UK2Node_FunctionEntry* FunctionEntry,
                                 UEdGraph* Graph) override;
};
