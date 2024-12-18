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
#include "RuleRangerObjectBase.h"
#include "UObject/Object.h"
#include "RuleRangerMatcher.generated.h"

/**
 * Base class used to match against an object to determine whether a rule should be applied to the object.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, CollapseCategories, DefaultToInstanced, EditInlineNew)
class RULERANGER_API URuleRangerMatcher : public URuleRangerObjectBase
{
    GENERATED_BODY()

public:
    /**
     * Inspect the object and return true if the object should be matched.
     *
     * @param Object the object to test.
     * @return true if the asset is a match, false otherwise.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Rule Ranger")
    bool Test(UObject* Object) const;
};
