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
#include "RuleRanger/RuleRangerUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Materials/MaterialInterface.h"

static const FName AssetToolsModuleName("AssetTools");

bool FRuleRangerUtilities::RenameAsset(UObject* Object, const FString& NewName)
{
    const auto PathName = Object->GetPathName();
    const auto PackagePath = FPackageName::GetLongPackagePath(Object->GetOutermost()->GetName());

    TArray<FAssetRenameData> AssetsAndNames;
    AssetsAndNames.Add(FAssetRenameData(Object, PackagePath, NewName));

    const bool bSuccess =
        FModuleManager::LoadModuleChecked<FAssetToolsModule>(AssetToolsModuleName).Get().RenameAssets(AssetsAndNames);

    // Notify asset registry of rename .. if required
    FAssetRegistryModule::AssetRenamed(Object, PathName);

    // This should not be called during loads of object so neither of these functions should return false
    ensure(Object->MarkPackageDirty());
    ensure(Object->GetOuter()->MarkPackageDirty());
    return bSuccess;
}

void FRuleRangerUtilities::CollectTypeHierarchy(const UObject* Object, TArray<UClass*>& Classes)
{
    bool bProcessedBlueprintHierarchy{ false };
    UClass* Class = Object->GetClass();
    while (Class)
    {
        if (!bProcessedBlueprintHierarchy && Object->IsA<UBlueprint>())
        {
            bProcessedBlueprintHierarchy = true;
            // If Object is a Blueprint then we have an alternate hierarchy accessible via the ParentClass property.
            // TODO: Assess with a similar pattern needs to be applied for other asset based class hierarchies...
            UClass* BlueprintClass{ Cast<UBlueprint>(Object)->ParentClass };
            while (BlueprintClass)
            {
                Classes.Add(BlueprintClass);
                BlueprintClass = BlueprintClass->GetSuperClass();
            }
        }
        Classes.Add(Class);
        Class = Class->GetSuperClass();
    }
}

bool FRuleRangerUtilities::IsA(const UObject* Object, const UClass* Class)
{
    if (Object->IsA(Class))
    {
        return true;
    }
    else
    {
        const UClass* ParentClass = Object->GetClass();
        while (ParentClass)
        {
            if (Object->IsA<UBlueprint>())
            {
                // If Object is a Blueprint then we have an alternate hierarchy accessible via the ParentClass property.
                if (const UClass * BlueprintClass{ Cast<UBlueprint>(Object)->ParentClass })
                {
                    return BlueprintClass->IsChildOf(Class);
                }
                return false;
            }
            ParentClass = ParentClass->GetSuperClass();
        }
        return false;
    }
}

void FRuleRangerUtilities::CollectInstanceHierarchy(UObject* Object, TArray<UObject*>& Instances)
{
    Instances.Add(Object);

    if (Object->IsA<UMaterialInstance>())
    {
        const UMaterialInstance* Instance = Cast<UMaterialInstance>(Object);
        while (Instance)
        {
            if (Instance->Parent)
            {
                Instances.Add(Instance->Parent);
            }
            // This will be null if we traverse from MaterialInstance parent to Material
            // which will terminate this loop
            Instance = Cast<UMaterialInstance>(Instance->Parent);
        }
    }
}
