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
#include "EnsureRequiredPropertiesPresentAction.generated.h"

/**
 * Action to check that any property with the (boolean) meta property 'RuleRangerRequired' is present on an object.
 * The Action will also look up the same property for the containing UCLASS macro and all transitive parents.
 * The property in the UCLASS macro is a comma separated list of required property names that are expected to be in
 * the form MyObject.MyProperty. Typically, this is used to validate the presence of properties on components.
 *
 * For example, you may add "UCLASS(meta = (RuleRangerRequired = "WeaponMesh.StaticMesh,ShieldMesh.StaticMesh"))"
 * to require the presence of two components WeaponMesh and ShieldMesh with valid values for StaticMesh.
 */
UCLASS(DisplayName = "Ensure Required Properties Present")
class RULERANGER_API UEnsureRequiredPropertiesPresentAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** Should the action skip checks for Abstract types. */
    UPROPERTY(EditAnywhere)
    bool bSkipCheckOnAbstractTypes{ true };

    void PerformChecksOnObject(URuleRangerActionContext* ActionContext, const UObject* Object) const;
    void PerformChecksForComponentProperties(URuleRangerActionContext* ActionContext,
                                             const UObject* Object,
                                             const TMap<FString, TArray<FString>>& RequiredComponentProperties);
    void PerformChecksForComponentProperties(URuleRangerActionContext* ActionContext,
                                             const UClass* Class,
                                             const FString& ComponentName,
                                             const UObject* Component,
                                             const TArray<FString>& PropertyNames);

    void DeriveRequiredComponentProperties(const UClass* Class,
                                           TMap<FString, TArray<FString>>& RequiredComponentProperties);

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;
};
