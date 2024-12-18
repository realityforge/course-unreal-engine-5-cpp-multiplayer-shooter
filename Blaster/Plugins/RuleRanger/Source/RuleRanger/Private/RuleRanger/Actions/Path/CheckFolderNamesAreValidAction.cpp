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
#include "CheckFolderNamesAreValidAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CheckFolderNamesAreValidAction)

UCheckFolderNamesAreValidAction::UCheckFolderNamesAreValidAction()
{
    InvalidNames.Add("New Folder");
    InvalidNames.Add("Assets");
    InvalidNames.Add("Asset");
}

void UCheckFolderNamesAreValidAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    TArray<FString> Folders;
    const FString PathName = Object->GetPackage()->GetPathName();
    if (PathName.StartsWith(TEXT("/Game/__ExternalActors__/"))
        || PathName.StartsWith(TEXT("/Game/__ExternalObjects__/")))
    {
        // Path is for an asset using OFPA pattern so are not user controller or user visible
        // and thus no need to verify
        return;
    }
    PathName.ParseIntoArray(Folders, TEXT("/"), true);

    // Pop the last element so we are just checking folder and not asset name
    Folders.Pop();

    const FRegexPattern Pattern(ValidFolderPattern,
                                bCaseSensitive ? ERegexPatternFlags::None : ERegexPatternFlags::CaseInsensitive);

    for (const auto& Folder : Folders)
    {
        if (InvalidNames.Contains(Folder) || !FRegexMatcher(Pattern, Folder).FindNext())
        {
            const auto& ErrorMessage = Message.IsEmpty()
                ? FString::Printf(TEXT("Asset is contained in a folder named '%s'"
                                       " with a full path '%s'. "
                                       "Folder names must match the pattern '%s' and "
                                       "must not be listed in invalid list: %s. Move "
                                       "the asset to a different folder."),
                                  *Folder,
                                  *PathName,
                                  *ValidFolderPattern,
                                  *FString::Join(InvalidNames, TEXT(", ")))
                : Message;
            if (Message.IsEmpty())
            {
                ActionContext->Error(FText::FromString(ErrorMessage));
            }
            else
            {
                ActionContext->Error(FText::FromString(Message));
            }
        }
    }
}
