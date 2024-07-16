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

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RuleRangerRuleSet.generated.h"

class URuleRangerRule;

/**
 * A named set of rules that can be for a project.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"), Blueprintable, BlueprintType, CollapseCategories, EditInlineNew)
class RULERANGER_API URuleRangerRuleSet final : public UDataAsset
{
    GENERATED_BODY()

public:
    /** A description of the rule set. */
    UPROPERTY(EditDefaultsOnly, Category = "Rule Ranger")
    FText Description;

    /** A set of RuleSets that this ruleSet aggregates. These rules are applied prior to the individual rules. */
    UPROPERTY(EditDefaultsOnly, Category = "Rules", meta = (AllowAbstract = "false", DisplayThumbnail = "false"))
    TArray<TObjectPtr<URuleRangerRuleSet>> RuleSets;

    /** A set of rules to be applied to different types. */
    UPROPERTY(EditDefaultsOnly, Category = "Rules", meta = (AllowAbstract = "false", DisplayThumbnail = "false"))
    TArray<TObjectPtr<URuleRangerRule>> Rules;
};
