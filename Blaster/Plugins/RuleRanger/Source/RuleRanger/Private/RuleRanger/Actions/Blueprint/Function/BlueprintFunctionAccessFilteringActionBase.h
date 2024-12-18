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

#include "BlueprintFunctionActionBase.h"
#include "CoreMinimal.h"
#include "K2Node_FunctionEntry.h"
#include "BlueprintFunctionAccessFilteringActionBase.generated.h"

/**
 * Abstract action to analyze functions that allows filtering based on access level.
 */
UCLASS(Abstract)
class RULERANGER_API UBlueprintFunctionAccessFilteringActionBase : public UBlueprintFunctionActionBase
{
    GENERATED_BODY()

    /** Should the action check private functions? */
    UPROPERTY(EditAnywhere)
    bool bCheckPrivateFunctions{ false };

    /** Should the action check protected functions? */
    UPROPERTY(EditAnywhere)
    bool bCheckProtectedFunctions{ true };

protected:
    virtual bool ShouldAnalyzeFunction(UBlueprint* Blueprint, UK2Node_FunctionEntry* FunctionEntry) const override;
};
