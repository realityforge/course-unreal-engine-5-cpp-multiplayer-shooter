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

#include "EnsureSkeletalMeshHasBones.h"

void UEnsureSkeletalMeshHasBones::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    const auto SkeletalMesh = CastChecked<USkeletalMesh>(Object);
    if (ensure(SkeletalMesh->GetSkeleton()))
    {
        const USkeleton* Skeleton = SkeletalMesh->GetSkeleton();
        for (const auto BoneName : Bones)
        {
            if (INDEX_NONE == Skeleton->GetReferenceSkeleton().FindBoneIndex(BoneName))
            {
                const auto OutMessage = FString::Printf(TEXT("Bone named '%s' does not exist on "
                                                             "the SkeletalMesh named '%s' with Skeleton named  "
                                                             "%s component. Reason: %s"),
                                                        *BoneName.ToString(),
                                                        *SkeletalMesh->GetFullName(),
                                                        *Skeleton->GetFullName(),
                                                        *Reason);
                ActionContext->Error(FText::FromString(OutMessage));
            }
        }
    }
}

UClass* UEnsureSkeletalMeshHasBones::GetExpectedType()
{
    return USkeletalMesh::StaticClass();
}
