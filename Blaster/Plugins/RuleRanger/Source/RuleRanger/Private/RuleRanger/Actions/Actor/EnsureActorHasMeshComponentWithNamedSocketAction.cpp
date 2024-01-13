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

#include "EnsureActorHasMeshComponentWithNamedSocketAction.h"
#include "RuleRanger/RuleRangerUtilities.h"

void UEnsureActorHasMeshComponentWithNamedSocketAction::EmitErrorMessage(URuleRangerActionContext* ActionContext,
                                                                         const TObjectPtr<UObject>& Asset) const
{
    const auto OutMessage = FString::Printf(TEXT("Socket named '%s' does not exist on "
                                                 "the mesh named '%s' assigned to the "
                                                 "%s component. Reason: %s"),
                                            *Socket.ToString(),
                                            Asset ? *Asset->GetFullName() : TEXT("None"),
                                            *ComponentName,
                                            *Reason);
    ActionContext->Error(FText::FromString(OutMessage));
}

void UEnsureActorHasMeshComponentWithNamedSocketAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                             UObject* Object)
{
    if (const auto& Actor = FRuleRangerUtilities::ToObject<AActor>(Object))
    {
        for (const auto Component : Actor->GetComponents())
        {
            if (Component->GetName().Equals(ComponentName))
            {
                if (const auto SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Component))
                {
                    if (!SkeletalMeshComponent->DoesSocketExist(Socket))
                    {
                        EmitErrorMessage(ActionContext, SkeletalMeshComponent->GetSkeletalMeshAsset());
                    }
                }
                else if (const auto StaticMeshComponent = Cast<UStaticMeshComponent>(Component))
                {
                    if (!StaticMeshComponent->DoesSocketExist(Socket))
                    {
                        EmitErrorMessage(ActionContext, StaticMeshComponent->GetStaticMesh());
                    }
                }
                else
                {
                    LogError(Object,
                             FString::Printf(TEXT("Action running on class %s that has a component named %s but the "
                                                  "component is not of the known types SkeletalMeshComponent or "
                                                  "StaticMeshComponent but is a type %s"),
                                             *Object->GetClass()->GetName(),
                                             *ComponentName,
                                             *Component->GetClass()->GetName()));
                }
                return;
            }
        }
        LogError(Object,
                 FString::Printf(TEXT("Action running on class %s has no component "
                                      "named %s so rule can not run."),
                                 *Object->GetClass()->GetName(),
                                 *ComponentName));
    }
    else
    {
        LogError(Object, TEXT("Action running on class that is not an Actor"));
    }
}

UClass* UEnsureActorHasMeshComponentWithNamedSocketAction::GetExpectedType()
{
    return AActor::StaticClass();
}
