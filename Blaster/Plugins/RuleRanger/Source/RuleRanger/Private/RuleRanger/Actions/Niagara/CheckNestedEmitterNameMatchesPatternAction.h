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
#include "RuleRangerAction.h"
#include "CheckNestedEmitterNameMatchesPatternAction.generated.h"

/**
 * Action to check that emitters contained in a Niagara System are named according to a regex.
 */
UCLASS(DisplayName = "Check Nested Emitter Name Matches Pattern")
class RULERANGER_API UCheckNestedEmitterNameMatchesPatternAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** The regex pattern to match. */
    UPROPERTY(EditAnywhere)
    FString Pattern{ TEXT("^[A-Z][a-z0-9A-Z_]*$") };
    /** A flag controlling whether matching is Case Sensitive or not. */
    UPROPERTY(EditAnywhere)
    bool bCaseSensitive{ true };

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() override;
};
