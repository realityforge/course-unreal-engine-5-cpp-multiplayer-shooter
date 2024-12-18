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

#include "RuleRangerRuleExclusion.generated.h"

class URuleRangerRule;
class URuleRangerRuleSet;

/**
 * Configuration indicating that one or more rules should be excluded when checking certain assets.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"),
       Blueprintable,
       BlueprintType,
       CollapseCategories,
       DefaultToInstanced,
       EditInlineNew)
class RULERANGER_API URuleRangerRuleExclusion final : public UDataAsset
{
    GENERATED_BODY()

public:
    /** A description indicating why the exclusion is in place. */
    UPROPERTY(EditDefaultsOnly, Category = "Rule Ranger")
    FText Description;

    /** A set of RuleSets that are excluded. */
    UPROPERTY(EditDefaultsOnly, Category = "Rules", meta = (AllowAbstract = "false", DisplayThumbnail = "false"))
    TArray<TObjectPtr<URuleRangerRuleSet>> RuleSets;

    /** A set of Rules that are excluded. */
    UPROPERTY(EditDefaultsOnly, Category = "Rules", meta = (AllowAbstract = "false", DisplayThumbnail = "false"))
    TArray<TObjectPtr<URuleRangerRule>> Rules;

    /** A set of objects that the exclusions apply to. */
    UPROPERTY(EditDefaultsOnly, Category = "Rules", meta = (AllowAbstract = "false", DisplayThumbnail = "false"))
    TArray<TObjectPtr<UObject>> Objects;

    /** A set of path prefixes that the exclusions apply to. */
    UPROPERTY(EditDefaultsOnly, Category = "Rules", meta = (ContentDir))
    TArray<FDirectoryPath> Dirs;

    bool ExclusionMatches(const UObject& Object, const FString& Path) const;
};
