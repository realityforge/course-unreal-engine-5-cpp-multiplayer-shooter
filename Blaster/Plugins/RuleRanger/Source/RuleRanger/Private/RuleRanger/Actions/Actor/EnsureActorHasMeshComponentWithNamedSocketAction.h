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
#include "EnsureActorHasMeshComponentWithNamedSocketAction.generated.h"

/**
 * Action to check that an actor class that contains either a SkeletalMeshComponent or StaticMeshComponent where the
 * referenced mesh asset has a socket with a specific name.
 *
 * This action is used create rules such as:
 * - Subclasses of Weapons have a mesh with a MuzzleFlash socket (To attach particle system to)
 * - Subclasses of ProjectileWeapon have a mesh with a MuzzleExit socket (To emit particles from)
 * - Subclasses of Weapon have a mesh with a LeftHand socket (To attach hands to using IK etc)
 */
UCLASS(DisplayName = "Ensure Actor Has Mesh Component With Named Socket")
class RULERANGER_API UEnsureActorHasMeshComponentWithNamedSocketAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** The name of the component that extends UMeshComponent that this rule checks. */
    UPROPERTY(EditAnywhere)
    FString ComponentName{ "" };

    /** The name of the socket that must exist on mesh. */
    UPROPERTY(EditAnywhere)
    FName Socket{ "" };

    /** The reason why we must have the socket. Used when emitting an error. */
    UPROPERTY(EditAnywhere)
    FString Reason{ "" };

    void EmitErrorMessage(URuleRangerActionContext* ActionContext, const TObjectPtr<UObject>& Asset) const;

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() override;
};
