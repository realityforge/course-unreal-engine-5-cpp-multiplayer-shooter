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
#include "AutomaticallySetUsageInEditorMaterialMatcher.generated.h"

/**
 * Matcher that matches if the object is a Material with bAutomaticallySetUsageInEditor set to specified value.
 */
UCLASS()
class RULERANGER_API UAutomaticallySetUsageInEditorMaterialMatcher final : public URuleRangerMatcher
{
    GENERATED_BODY()

    /** The expected value of AutomaticallySetUsageInEditor flag. */
    UPROPERTY(EditDefaultsOnly)
    bool bAutomaticallySetUsageInEditor{ true };

public:
    bool Test_Implementation(UObject* Object) const override;
};
