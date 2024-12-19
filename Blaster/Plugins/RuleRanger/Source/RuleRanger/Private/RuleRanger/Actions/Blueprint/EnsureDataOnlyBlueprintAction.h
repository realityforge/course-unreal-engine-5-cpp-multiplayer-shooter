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
#include "EnsureDataOnlyBlueprintAction.generated.h"

/**
 * Action to check that Blueprints assets that extend specific types are DataOnlyBlueprints.
 */
UCLASS(DisplayName = "Ensure Blueprints derived from specific types are Data Only Blueprints")
class RULERANGER_API UEnsureDataOnlyBlueprintAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** The types where subtypes MUST be DataOnlyBlueprints. */
    UPROPERTY(EditAnywhere, meta = (AllowAbstract = true))
    TArray<TSubclassOf<UObject>> ObjectTypes;

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() override;
};
