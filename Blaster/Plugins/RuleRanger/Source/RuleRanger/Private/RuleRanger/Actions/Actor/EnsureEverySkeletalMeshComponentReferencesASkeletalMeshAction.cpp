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

#include "EnsureEverySkeletalMeshComponentReferencesASkeletalMeshAction.h"

void UEnsureEverySkeletalMeshComponentReferencesASkeletalMeshAction::Apply_Implementation(
    URuleRangerActionContext* ActionContext,
    UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto Actor = CastChecked<AActor>(Object);
    TArray<USkeletalMeshComponent*> Components;
    Actor->GetComponents<USkeletalMeshComponent>(Components);
    for (const auto Component : Components)
    {
        if (!Component->GetSkeletalMeshAsset())
        {
            const auto OutMessage =
                FString::Printf(TEXT("SkeletalMeshComponent named '%s' does not reference a valid SkeletalMesh"),
                                *Component->GetName());
            ActionContext->Error(FText::FromString(OutMessage));
        }
    }
}

UClass* UEnsureEverySkeletalMeshComponentReferencesASkeletalMeshAction::GetExpectedType()
{
    return AActor::StaticClass();
}
