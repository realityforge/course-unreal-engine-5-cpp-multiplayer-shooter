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

#include "CheckLightMapUVsAction.h"

void UCheckLightMapUVsAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    UStaticMesh* StaticMesh = CastChecked<UStaticMesh>(Object);

    TArray<FString> MissingUVs;
    TArray<FString> BadUVs;
    TArray<FString> ValidUVs;

    UStaticMesh::CheckLightMapUVs(StaticMesh, MissingUVs, BadUVs, ValidUVs, true);

    if (MissingUVs.Num() > 0)
    {
        const auto& ErrorMessage =
            FString::Printf(TEXT("StaticMesh is missing UVs: %s"), *FString::Join(MissingUVs, TEXT(", ")));
        ActionContext->Error(FText::FromString(ErrorMessage));
    }
    if (BadUVs.Num() > 0)
    {
        const auto& ErrorMessage =
            FString::Printf(TEXT("StaticMesh invalid UVs: %s"), *FString::Join(BadUVs, TEXT(", ")));
        ActionContext->Error(FText::FromString(ErrorMessage));
    }
}

UClass* UCheckLightMapUVsAction::GetExpectedType()
{
    return UStaticMesh::StaticClass();
}
