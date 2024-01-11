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

#include "BaseAnalyzeFunctionAction.h"
#include "CoreMinimal.h"
#include "RuleRangerAction.h"
#include "EnsureFunctionsNameMatchesRegexAction.generated.h"

/**
 * Action to ensure that the names of functions match specified regex.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"))
class RULERANGER_API UEnsureFunctionsNameMatchesRegexAction final : public UBaseAnalyzeFunctionAction
{
    GENERATED_BODY()

    /** The regex pattern that function is expected to match. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    FString Pattern{ TEXT("") };
    /** A flag controlling whether matching is Case Sensitive or not. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    bool bCaseSensitive{ true };

protected:
    virtual void AnalyzeFunction(URuleRangerActionContext* ActionContext,
                                 UBlueprint* Blueprint,
                                 UK2Node_FunctionEntry* FunctionEntry,
                                 UEdGraph* Graph) override;
};
