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

#include "RemoveMetadataTagsAction.h"
#include "Editor.h"
#include "Subsystems/EditorAssetSubsystem.h"

void URemoveMetadataTagsAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>())
    {
        for (const auto& MetadataKey : Keys)
        {
            if (NAME_None == MetadataKey)
            {
                LogError(Object, TEXT("Empty key specified when attempting to remove MetadataTag."));
            }
            else
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                FString ExistingValue = Subsystem->GetMetadataTag(Object, MetadataKey);
                if (ExistingValue.Equals(TEXT("")))
                {
                    LogInfo(Object,
                            FString::Printf(TEXT("MetaData with key %s does not exist on object. No action required"),
                                            *MetadataKey.ToString()));
                }
                else
                {
                    if (ActionContext->IsDryRun())
                    {
                        FFormatNamedArguments Arguments;
                        Arguments.Add(TEXT("Key"), FText::FromString(MetadataKey.ToString()));
                        const FText Message =
                            FText::Format(NSLOCTEXT("RuleRanger",
                                                    "MetaDataTagRemoveOmitted",
                                                    "MetaData tag {Key} is present. This tag would be removed if "
                                                    "RuleRanger was not in DryRun mode"),
                                          Arguments);

                        ActionContext->Error(Message);
                    }
                    else
                    {
                        FFormatNamedArguments Arguments;
                        Arguments.Add(TEXT("Key"), FText::FromString(MetadataKey.ToString()));
                        const FText Message =
                            FText::Format(NSLOCTEXT("RuleRanger",
                                                    "RemovingMetaDataTag",
                                                    "MetaData tag with key {Key} is present. Removing tag."),
                                          Arguments);

                        ActionContext->Info(Message);
                        Subsystem->RemoveMetadataTag(Object, MetadataKey);
                        // This should not be called during loads of object so neither of these functions should
                        // return false
                        ensure(Object->MarkPackageDirty());
                        ensure(Object->GetOuter()->MarkPackageDirty());
                    }
                }
            }
        }
    }
}
