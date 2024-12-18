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
#include "RuleRangerConfig.generated.h"

class URuleRangerRuleExclusion;
class URuleRangerRuleSet;

/**
 * A configuration which applies RuleSets to a set of Content Directories.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"), Blueprintable, BlueprintType, CollapseCategories, EditInlineNew)
class RULERANGER_API URuleRangerConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    /** The base content directory to which the RuleSet applies. */
    UPROPERTY(EditDefaultsOnly, Category = "Rule Ranger", meta = (ContentDir))
    TArray<FDirectoryPath> Dirs;

    /**
     * A set of DataTables that are used to drive different actions.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Rule Sets", meta = (DisplayThumbnail = "false"))
    TArray<TObjectPtr<UDataTable>> DataTables;

    /** A set of rule sets to be applied to directories. */
    UPROPERTY(EditDefaultsOnly, Category = "Rule Sets", meta = (AllowAbstract = "false", DisplayThumbnail = "false"))
    TArray<TObjectPtr<URuleRangerRuleSet>> RuleSets;

    /** A set of exclusions when applying rules. */
    UPROPERTY(EditDefaultsOnly, Category = "Rule Sets", meta = (AllowAbstract = "false", DisplayThumbnail = "false"))
    TArray<TObjectPtr<URuleRangerRuleExclusion>> Exclusions;

    bool ConfigMatches(const FString& Path);
};
