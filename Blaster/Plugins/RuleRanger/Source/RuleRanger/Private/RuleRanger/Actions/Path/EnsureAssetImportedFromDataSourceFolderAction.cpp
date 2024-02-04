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

#include "EnsureAssetImportedFromDataSourceFolderAction.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "EditorFramework/AssetImportData.h"
#include "UObject/PropertyAccessUtil.h"

void UEnsureAssetImportedFromDataSourceFolderAction::ValidateAssetImportData(
    URuleRangerActionContext* ActionContext,
    const FString& DataSourceFolder,
    const UObject* Object,
    const UAssetImportData* const AssetImportData) const
{
    for (const auto& File : AssetImportData->SourceData.SourceFiles)
    {
        // Absolute path ala "C:/MyProject/SourceArt/Character/Crypto/Textures/T_Crypto_N.png"
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto& ImportFilename =
            UAssetImportData::ResolveImportFilename(File.RelativeFilename, Object->GetOutermost());
        if (!ImportFilename.StartsWith(DataSourceFolder))
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("Filename"), FText::FromString(ImportFilename));
            Arguments.Add(TEXT("DataSourceFolder"), FText::FromString(DataSourceFolder));

            ActionContext->Error(
                FText::Format(NSLOCTEXT("RuleRanger",
                                        "EnsureAssetImportedFromDataSourceFolderAction_OutsideDataSourceFolder",
                                        "Asset imported from a file '{Filename}' that "
                                        "is outside of the Data Source Folder "
                                        "'{DataSourceFolder}'."),
                              Arguments));
        }
        else
        {
            // Filename relative to DataSourceFolder ala "Character/Crypto/Textures/T_Crypto_N.png"
            FString RelativeFilename{ ImportFilename };
            RelativeFilename.RightChopInline(DataSourceFolder.Len() + 1);

            // Path relative to DataSourceFolder "Character/Crypto/Textures"
            const FString RelativePath = FPaths::GetPath(RelativeFilename);

            // Base name of file ala "T_Crypto_N"
            const FString BaseName = FPaths::GetBaseFilename(RelativeFilename);

            LogError(Object,
                     FString::Printf(TEXT("Object imported from '%s', (relative path = '%s' base name = '%s')"),
                                     *ImportFilename,
                                     *RelativePath,
                                     *BaseName));

            const FString ObjectPathName{ FPaths::GetPath(
                Object->GetOutermost()->GetPathName().RightChop(6 /* size of '/Game/' */)) };
            const FString ObjectName{ Object->GetName() };

            LogInfo(Object,
                    FString::Printf(TEXT("Evaluating object with "
                                         "path '%s' and name '%s'"),
                                    *ObjectPathName,
                                    *ObjectName));

            if (bRequireMatchingName)
            {
                if (!ObjectName.Equals(BaseName))
                {
                    FFormatNamedArguments Arguments;
                    Arguments.Add(TEXT("ObjectName"), FText::FromString(ObjectName));
                    Arguments.Add(TEXT("BaseName"), FText::FromString(BaseName));
                    Arguments.Add(TEXT("ImportFilename"), FText::FromString(ImportFilename));

                    ActionContext->Error(
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "EnsureAssetImportedFromDataSourceFolderAction_NameNoMatch",
                                                "Asset imported from a file '{ImportFilename}' with a base "
                                                "filename '{BaseName}' that does not match the asset "
                                                "name '{ObjectName}' as expected"),
                                      Arguments));
                }
            }

            if (bRequireMatchingPath)
            {
                if (!ObjectPathName.Equals(RelativePath))
                {
                    FFormatNamedArguments Arguments;
                    Arguments.Add(TEXT("ObjectPathName"), FText::FromString(ObjectPathName));
                    Arguments.Add(TEXT("RelativePath"), FText::FromString(RelativePath));
                    Arguments.Add(TEXT("ImportFilename"), FText::FromString(ImportFilename));

                    ActionContext->Error(
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "EnsureAssetImportedFromDataSourceFolderAction_PathNoMatch",
                                                "Asset imported from a file '{ImportFilename}' with a data "
                                                "source path '{RelativePath}' that does not match the asset "
                                                "path '{ObjectPathName}' as expected"),
                                      Arguments));
                }
            }
        }
    }
}

void UEnsureAssetImportedFromDataSourceFolderAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                          UObject* Object)
{
    static const FName PropertyName = FName("AssetImportData");
    const UEditorPerProjectUserSettings* EditorSettings = GetDefault<UEditorPerProjectUserSettings>();
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto& DataSourceFolder = EditorSettings->DataSourceFolder.Path;
    if (DataSourceFolder.IsEmpty())
    {
        ActionContext->Error(NSLOCTEXT("RuleRanger",
                                       "EnsureAssetImportedFromDataSourceFolderAction_MissingDataSourceFolder",
                                       "Data Source Folder not set. Please set it in Editor Preferences"));
    }
    else if (const auto ObjectProp = PropertyAccessUtil::FindPropertyByName(PropertyName, Object->GetClass()))
    {
        void* Value{ nullptr };

        // ReSharper disable once CppTooWideScopeInitStatement
        const auto Result =
            PropertyAccessUtil::GetPropertyValue_Object(ObjectProp, Object, ObjectProp, &Value, INDEX_NONE);
        if (EPropertyAccessResultFlags::Success == Result)
        {
            // ReSharper disable once CppTooWideScopeInitStatement
            const auto AssetImportData = static_cast<const UAssetImportData*>(Value);
            if (AssetImportData && AssetImportData->SourceData.SourceFiles.Num() > 0)
            {
                if (!Object->GetOutermost()->GetName().StartsWith(TEXT("/Game/")))
                {
                    LogInfo(Object, TEXT("Object is outside of /Game hierarchy, skipping analysis"));
                }
                else
                {
                    ValidateAssetImportData(ActionContext, DataSourceFolder, Object, AssetImportData);
                }
            }
            else if (!bSkipAssetsMissingAssetImportData)
            {
                ActionContext->Error(FText::FromString(L"Asset missing AssetImportData and "
                                                       "SkipAssetsMissingAssetImportData=false. Change setting "
                                                       "or ensure asset has been imported correctly."));
            }
        }
        else
        {
            ActionContext->Error(FText::FromString(TEXT("Failed to access AssetImportData property")));
        }
    }
}
