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
#include "UObject/Object.h"
#include "RuleRangerDeveloperSettings.generated.h"

class URuleRangerConfig;

/**
 * RuleRanger Project Settings in the Editor category.
 */
UCLASS(Config = "Editor", DefaultConfig, DisplayName = "Rule Ranger")
class RULERANGER_API URuleRangerDeveloperSettings final : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /** Gets the settings container name for the settings, either Project or Editor */
    virtual FName GetContainerName() const override { return FName("Project"); }
    /** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
    virtual FName GetCategoryName() const override { return FName("Editor"); }

    /** Should we always show the message log after a request to scan assets or only when warnings or errors occur? */
    UPROPERTY(Config, EditAnywhere, Category = "Rule Ranger", meta = (DisplayThumbnail = "false"))
    bool bAlwaysShowMessageLog;

    UPROPERTY(Config, EditAnywhere, Category = "Rule Ranger", meta = (DisplayThumbnail = "false"))
    TArray<TSoftObjectPtr<URuleRangerConfig>> Configs;
};
