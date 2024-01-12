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
#include "EnsureVariablesNameMatchesRegexAction.generated.h"

/**
 * Action to check that the variable defined in the Blueprint match a pattern.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"))
class RULERANGER_API UEnsureVariablesNameMatchesRegexAction final : public UBaseAnalyzeVariableAction
{
    GENERATED_BODY()

    /** The regex pattern that the variable name is expected to match. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    FString Pattern{ TEXT("^[A-Z][a-z0-9A-Z_]*$") };
    /** A flag controlling whether matching is Case Sensitive or not. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    bool bCaseSensitive{ true };

protected:
    virtual void AnalyzeVariable(URuleRangerActionContext* ActionContext,
                                 UBlueprint* Blueprint,
                                 const FBPVariableDescription& Variable,
                                 UEdGraph* Graph) override;
};
