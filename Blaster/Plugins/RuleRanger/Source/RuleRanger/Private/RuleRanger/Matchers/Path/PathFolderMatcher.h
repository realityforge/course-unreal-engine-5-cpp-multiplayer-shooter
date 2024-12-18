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
#include "RuleRangerMatcher.h"
#include "UObject/Object.h"
#include "PathFolderMatcher.generated.h"

/**
 * Matcher that returns true if object has a path that contains a folder matched by one of the rules.
 */
UCLASS()
class RULERANGER_API UPathFolderMatcher final : public URuleRangerMatcher
{
    GENERATED_BODY()

    /** The Folders to match. */
    UPROPERTY(EditAnywhere)
    TArray<FString> FolderNames;
    /** The regex pattern to match. */
    UPROPERTY(EditAnywhere)
    FString RegexPattern{ TEXT("^(\\/[a-zA-Z][a-zA-Z0-9_]*)+$") };
    /** A flag controlling whether regex matching is Case Sensitive or not. */
    UPROPERTY(EditAnywhere)
    bool bCaseSensitive{ true };

public:
    bool Test_Implementation(UObject* Object) const override;
};
