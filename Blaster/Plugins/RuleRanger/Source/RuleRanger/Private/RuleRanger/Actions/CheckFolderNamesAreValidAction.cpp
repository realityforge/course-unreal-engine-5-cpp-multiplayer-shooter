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

void UCheckFolderNamesAreValidAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    TArray<FString> Folders;
    Object->GetPathName().ParseIntoArray(Folders, TEXT("/"), true);

    const FRegexPattern Pattern(InvalidFolderRegexPattern,
                                bCaseSensitive ? ERegexPatternFlags::None : ERegexPatternFlags::CaseInsensitive);

    for (const auto& Folder : Folders)
    {
        if (InvalidFolderNames.Contains(Folder)
            || (!InvalidFolderRegexPattern.IsEmpty() && FRegexMatcher(Pattern, Folder).FindNext()))
        {
            const auto& ErrorMessage = Message.IsEmpty()
                ? FString::Printf(TEXT("Asset is contained in a folder '%s' with "
                                       "an invalid name. Move the asset to a different folder."),
                                  *Folder)
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
