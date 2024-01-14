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

#include "CheckTopLevelFolderContentIsValidAction.h"

void UCheckTopLevelFolderContentIsValidAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                    UObject* Object)
{
    TArray<FString> Folders;
    Object->GetPathName().ParseIntoArray(Folders, TEXT("/"), true);
    if (!Folders[0].Equals(TEXT("Game")))
    {
        LogError(Object, TEXT("Object is not under /Game. The action does not support this scenario."));
        return;
    }

    const FRegexPattern ValidAssetPattern(ValidAssetRegexPattern,
                                          bCaseSensitive ? ERegexPatternFlags::None
                                                         : ERegexPatternFlags::CaseInsensitive);
    const FRegexPattern ValidFolderPattern(ValidFolderRegexPattern,
                                           bCaseSensitive ? ERegexPatternFlags::None
                                                          : ERegexPatternFlags::CaseInsensitive);

    const auto& Item = Folders[1];
    LogInfo(Object, FString::Printf(TEXT("Checking that the top-level name %s"), *Item));
    // Index 0 is "Game" (or Engine or the plugin name)
    if (2 == Folders.Num())
    {
        if (!ValidAssetNames.Contains(Item)
            && !(!ValidAssetRegexPattern.IsEmpty() && FRegexMatcher(ValidAssetPattern, Item).FindNext()))
        {
            const auto& ErrorMessage = Message.IsEmpty()
                ? FString::Printf(
                    TEXT("Asset exists at the root of the content directory. "
                         "The named of the asset must be one of [%s]%s."),
                    *FString::Join(ValidAssetNames, TEXT(", ")),
                    ValidAssetRegexPattern.IsEmpty()
                        ? TEXT("")
                        : *FString::Printf(TEXT(" or the asset name must match regex %s"), *ValidAssetRegexPattern))
                : Message;
            ActionContext->Error(FText::FromString(ErrorMessage));
        }
    }
    else
    {
        if (!ValidFolderNames.Contains(Item)
            && !(!ValidFolderRegexPattern.IsEmpty() && FRegexMatcher(ValidFolderPattern, Item).FindNext()))
        {
            const auto& ErrorMessage = Message.IsEmpty()
                ? FString::Printf(
                    TEXT("Asset is contained in a top level folder named '%s'. "
                         "The top level folder must be one of [%s]%s."),
                    *Item,
                    *FString::Join(ValidFolderNames, TEXT(", ")),
                    ValidFolderRegexPattern.IsEmpty()
                        ? TEXT("")
                        : *FString::Printf(TEXT(" or the folder name must match regex %s"), *ValidFolderRegexPattern))
                : Message;
            ActionContext->Error(FText::FromString(ErrorMessage));
        }
    }
}
