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
#include "CheckNiagaraSystemCompileStatusAction.generated.h"

class UNiagaraScript;

/**
 * Action to check that the compile status of a Niagara System is valid.
 */
UCLASS(DisplayName = "Check Niagara System Compile Status")
class RULERANGER_API UCheckNiagaraSystemCompileStatusAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** Should the action generate an error on "Unknown" Compile Status. */
    UPROPERTY(EditAnywhere)
    bool bErrorOnUnknown{ true };

    /** Should the action generate an error on "UpToDateWithWarnings" Compile Status. */
    UPROPERTY(EditAnywhere)
    bool bErrorOnUpToDateWithWarnings{ true };

    /**
     * Validate that the script has been correctly compiled.
     *
     * @param ActionContext The ActionContext
     * @param Object The Object being validated
     * @param ContainerContext The description of the container used in error messages. (Scripts can be in the system or
     * one of many emitters)
     * @param Script The Script to validate
     * @return True if the script is valid, false otherwise
     */
    bool ValidateScript(URuleRangerActionContext* ActionContext,
                        const UObject* Object,
                        const FString& ContainerContext,
                        UNiagaraScript* Script) const;

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() override;
};
