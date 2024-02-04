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
#include "Engine/DataTable.h"
#include "RuleRangerAction.h"
#include "RemoveNameSuffixAction.generated.h"

/**
 * Action to remove a suffix from a name.
 */
UCLASS(DisplayName = "Remove Name Suffix")
class RULERANGER_API URemoveNameSuffixAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** The suffix to remove. */
    UPROPERTY(EditAnywhere)
    FString Suffix;
    /** A flag controlling whether matching is Case Sensitive or not. */
    UPROPERTY(EditAnywhere)
    bool bCaseSensitive{ true };

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;
};
