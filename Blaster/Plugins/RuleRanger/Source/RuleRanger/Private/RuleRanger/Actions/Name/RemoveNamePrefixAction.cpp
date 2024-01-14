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

#include "RemoveNamePrefixAction.h"
#include "Editor.h"
#include "RuleRanger/RuleRangerUtilities.h"

void URemoveNamePrefixAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    if (Prefix.IsEmpty())
    {
        LogError(Object, TEXT("Empty Prefix specified when attempting to remove Prefix."));
    }
    else
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        const FString OriginalName{ Object->GetName() };
        if (OriginalName.StartsWith(Prefix, bCaseSensitive ? ESearchCase::CaseSensitive : ESearchCase::IgnoreCase))
        {
            const FString NewName{ OriginalName.RightChop(Prefix.Len()) };
            if (ActionContext->IsDryRun())
            {
                FFormatNamedArguments Arguments;
                Arguments.Add(TEXT("OriginalName"), FText::FromString(OriginalName));
                Arguments.Add(TEXT("NewName"), FText::FromString(NewName));
                const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                              "ObjectRenameOmitted",
                                                              "Object needs to be renamed from '{OriginalName}' "
                                                              "to '{NewName}'. Action skipped in DryRun mode"),
                                                    Arguments);

                ActionContext->Warning(Message);
            }
            else
            {
                FFormatNamedArguments Arguments;
                Arguments.Add(TEXT("OriginalName"), FText::FromString(OriginalName));
                Arguments.Add(TEXT("NewName"), FText::FromString(NewName));
                const auto Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                             "ObjectRenamed",
                                                             "Object named {OriginalName} has been renamed "
                                                             "to {NewName} to match convention."),
                                                   Arguments);

                ActionContext->Info(Message);

                if (!FRuleRangerUtilities::RenameAsset(Object, NewName))
                {
                    const auto InMessage = FText::Format(NSLOCTEXT("RuleRanger",
                                                                   "ObjectRenameFailed",
                                                                   "Attempt to rename object '{0}' to '{1}' failed."),
                                                         FText::FromString(OriginalName),
                                                         FText::FromString(NewName));
                    ActionContext->Error(InMessage);
                }
            }
        }
    }
}
