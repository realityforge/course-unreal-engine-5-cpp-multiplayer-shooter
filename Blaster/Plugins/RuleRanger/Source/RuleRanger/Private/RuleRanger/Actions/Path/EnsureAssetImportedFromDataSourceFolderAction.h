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
#include "EnsureAssetImportedFromDataSourceFolderAction.generated.h"

/**
 * Action to check that an asset has been imported from the Data Source Folder (DSF).
 *  - (optional) Assets imported from the DSF have paths that match between import and content folders.
 *  - (optional) Assets imported from the DSF have filenames that match between import and content folders.
 *
 *  The Data Source Folder must be set in the Editor Preferences.
 */
UCLASS(DisplayName = "Ensure Asset Imported From Data Source Folder")
class RULERANGER_API UEnsureAssetImportedFromDataSourceFolderAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** Should the action skip assets missing AssetImportData? */
    UPROPERTY(EditAnywhere)
    bool bSkipAssetsMissingAssetImportData{ true };

    /** Should the path in the data source folder and content folder match? */
    UPROPERTY(EditAnywhere)
    bool bRequireMatchingPath{ true };

    /** Should the filename in the data source folder and content folder match? */
    UPROPERTY(EditAnywhere)
    bool bRequireMatchingName{ true };

    void ValidateAssetImportData(URuleRangerActionContext* ActionContext,
                                 const FString& DataSourceFolder,
                                 const UObject* Object,
                                 const UAssetImportData* AssetImportData) const;

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;
};
