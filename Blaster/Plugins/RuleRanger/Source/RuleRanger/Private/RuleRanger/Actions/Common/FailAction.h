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
#include "FailAction.generated.h"

/**
 * Action that always fails with specified message.
 *
 * Useful when using matchers to identify scenarios that you do not want in your codebase.
 */
UCLASS(DisplayName = "Fail")
class RULERANGER_API UFailAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** The error message to emit */
    UPROPERTY(EditAnywhere, meta = (MultiLine))
    FString Message{ "The object was not expected to match" };
    /** Should the error be a fatal error or a regular error */
    UPROPERTY(EditAnywhere)
    bool bFatal{ false };

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;
};
