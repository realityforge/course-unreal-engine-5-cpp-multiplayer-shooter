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
#include "Engine/DeveloperSettings.h"
#include "RuleRangerRuleSet.h"
#include "UObject/Object.h"
#include "RuleRangerDeveloperSettings.generated.h"

class URuleRangerRuleSetScope;

/**
 * RuleRanger Project Settings in the Editor category.
 */
UCLASS(Config = "Editor", DefaultConfig, DisplayName = "Rule Ranger")
class RULERANGER_API URuleRangerDeveloperSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /** Gets the settings container name for the settings, either Project or Editor */
    virtual FName GetContainerName() const override { return FName("Project"); }
    /** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
    virtual FName GetCategoryName() const override { return FName("Editor"); }

    UPROPERTY(Config, EditAnywhere, Category = "Rule Ranger", meta = (DisplayThumbnail = "false"))
    TArray<TSoftObjectPtr<URuleRangerRuleSetScope>> RuleSetScopes;
};
