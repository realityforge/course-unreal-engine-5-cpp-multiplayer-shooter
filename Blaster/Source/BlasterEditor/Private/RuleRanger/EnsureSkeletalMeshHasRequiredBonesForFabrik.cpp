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

#include "EnsureSkeletalMeshHasRequiredBonesForFabrik.h"

void UEnsureSkeletalMeshHasRequiredBonesForFabrik::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                        UObject* Object)
{
    if (IsValid(Object))
    {
        if (const auto SkeletalMesh = Cast<USkeletalMesh>(Object))
        {
            if (ensure(SkeletalMesh->GetSkeleton()))
            {
                const USkeleton* Skeleton = SkeletalMesh->GetSkeleton();
                if (INDEX_NONE == Skeleton->GetReferenceSkeleton().FindBoneIndex("hand_r"))
                {
                    ActionContext->Error(FText::FromString(
                        "Missing bone named hand_r on Skeletal mesh which is required for FABRIK system"));
                }
                if (INDEX_NONE == Skeleton->GetReferenceSkeleton().FindBoneIndex("hand_l"))
                {
                    ActionContext->Error(FText::FromString(
                        "Missing bone named hand_l on Skeletal mesh which is required for FABRIK system"));
                }
                if (INDEX_NONE == Skeleton->GetReferenceSkeleton().FindBoneIndex("upperarm_l"))
                {
                    ActionContext->Error(FText::FromString(
                        "Missing bone named upperarm_l on Skeletal mesh which is required for FABRIK system"));
                }
            }
        }
        else
        {
            LogError(Object, TEXT(""));
        }
    }
}
