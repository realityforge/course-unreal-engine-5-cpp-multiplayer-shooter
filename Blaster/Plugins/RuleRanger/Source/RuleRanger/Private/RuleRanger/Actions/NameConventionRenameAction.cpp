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

#include "NameConventionRenameAction.h"
#include "Editor.h"
#include "Misc/UObjectToken.h"
#include "RuleRanger/RuleRangerUtilities.h"
#include "RuleRangerMessageLog.h"
#include "Subsystems/EditorAssetSubsystem.h"

void UNameConventionRenameAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    if (IsValid(Object))
    {
        if (IsValid(NameConventionsTable))
        {
            RebuildNameConventionsCacheIfNecessary();

            if (!NameConventionsCache.IsEmpty())
            {
                const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
                const auto Variant =
                    Subsystem ? Subsystem->GetMetadataTag(Object, FName("RuleRanger.Variant")) : TEXT("");

                const FString OriginalName{ Object->GetName() };

                TArray<UClass*> Classes;
                FRuleRangerUtilities::CollectTypeHierarchy(Object, Classes);
                for (auto Class : Classes)
                {
                    LogInfo(
                        Object,
                        FString::Printf(TEXT("Looking for NamingConvention rules for class %s"), *Class->GetName()));
                    if (TArray<FNameConvention>* NameConventions = NameConventionsCache.Find(Class))
                    {
                        LogInfo(Object,
                                FString::Printf(TEXT("Found %d NamingConvention rules for %s"),
                                                NameConventions->Num(),
                                                *Class->GetName()));
                        for (int i = 0; i < NameConventions->Num(); i++)
                        {
                            const FNameConvention& NameConvention = (*NameConventions)[i];
                            LogInfo(
                                Object,
                                FString::Printf(TEXT("Attempting to match NameConvention "
                                                     "Prefix=%s, Suffix=%s, Variant=%s against asset with Variant=%s"),
                                                *NameConvention.Prefix,
                                                *NameConvention.Suffix,
                                                *NameConvention.Variant,
                                                *Variant));
                            if (NameConvention.Variant.Equals(Variant)
                                || NameConvention.Variant.Equals(NameConvention_DefaultVariant))
                            {
                                FString NewName{ OriginalName };
                                if (!NameConvention.Prefix.IsEmpty() && !NewName.StartsWith(NameConvention.Prefix))
                                {
                                    NewName.InsertAt(0, NameConvention.Prefix);
                                }
                                if (!NameConvention.Suffix.IsEmpty() && !NewName.EndsWith(NameConvention.Suffix))
                                {
                                    NewName.Append(NameConvention.Suffix);
                                }
                                if (NewName.Equals(OriginalName))
                                {
                                    LogInfo(Object, TEXT("Object matches naming convention. No action required."));
                                }
                                else
                                {
                                    if (ActionContext->IsDryRun())
                                    {
                                        FFormatNamedArguments Arguments;
                                        Arguments.Add(TEXT("OriginalName"), FText::FromString(OriginalName));
                                        Arguments.Add(TEXT("NewName"), FText::FromString(NewName));
                                        const FText Message =
                                            FText::Format(NSLOCTEXT("RuleRanger",
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
                                        const auto Message =
                                            FText::Format(NSLOCTEXT("RuleRanger",
                                                                    "ObjectRenamed",
                                                                    "Object named {OriginalName} has been renamed "
                                                                    "to {NewName} to match convention."),
                                                          Arguments);

                                        ActionContext->Info(Message);

                                        if (!FRuleRangerUtilities::RenameAsset(Object, NewName))
                                        {
                                            const auto InMessage = FText::Format(
                                                NSLOCTEXT("RuleRanger",
                                                          "ObjectRenameFailed",
                                                          "Attempt to rename object '{0}' to '{1}' failed."),
                                                FText::FromString(OriginalName),
                                                FText::FromString(NewName));
                                            ActionContext->Error(InMessage);
                                        }
                                    }
                                }
                                return;
                            }
                        }
                    }
                }

                // Only attempt to apply naming conventions to outermost packages
                if (const UObject* OutermostObject = Object->GetOutermostObject(); OutermostObject == Object)
                {
                    if (bNotifyIfNameConventionMissing)
                    {
                        FMessageLog(FRuleRangerMessageLog::GetMessageLogName())
                            .Warning()
                            ->AddToken(FTextToken::Create(NSLOCTEXT("RuleRanger",
                                                                    "MissingNamingConvention",
                                                                    "Unable to locate Naming Convention for")))
                            ->AddToken(FUObjectToken::Create(Object))
                            ->AddToken(FTextToken::Create(NSLOCTEXT("RuleRanger", "OfType", "of type")))
                            ->AddToken(FUObjectToken::Create(Object->GetClass()))
                            ->AddToken(FTextToken::Create(NSLOCTEXT("RuleRanger", "In", " in ")))
                            ->AddToken(FUObjectToken::Create(NameConventionsTable));
                    }
                    else
                    {
                        LogInfo(Object,
                                FString::Printf(TEXT("Unable to locate Naming Convention for "
                                                     "asset of type '%s' in '%s'."),
                                                *Object->GetClass()->GetName(),
                                                *NameConventionsTable->GetName()));
                    }
                }
            }
        }
        else
        {
            LogError(Object, TEXT("Action can not run as has not specified NameConventionsTable property."));
        }
    }
}

void UNameConventionRenameAction::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    // ReSharper disable once CppTooWideScopeInitStatement
    const FName TableName = GET_MEMBER_NAME_CHECKED(UNameConventionRenameAction, NameConventionsTable);
    if (TableName == PropertyName)
    {
        ResetNameConventionsCache();
    }
    Super::PostEditChangeProperty(PropertyChangedEvent);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UNameConventionRenameAction::ResetCacheIfTableModified(UObject* Object)
{
    // This is called on any object edit in editor so match against conventions table and bust cache as appropriate;
    if (Object && Object == NameConventionsTable)
    {
        ResetNameConventionsCache();
    }
}

void UNameConventionRenameAction::ResetNameConventionsCache()
{
    LogInfo(nullptr, TEXT("Resetting the Name Conventions Cache"));

    NameConventionsCache.Empty();
    FCoreUObjectDelegates::OnObjectModified.Remove(OnObjectModifiedDelegateHandle);
    OnObjectModifiedDelegateHandle.Reset();
}

void UNameConventionRenameAction::RebuildNameConventionsCacheIfNecessary()
{
    if (NameConventionsCache.IsEmpty() && 0 != NameConventionsTable->GetTableData().Num())
    {
        ResetNameConventionsCache();
        // Add a callback for when ANY object is modified in the editor so that we can bust the cache
        OnObjectModifiedDelegateHandle =
            FCoreUObjectDelegates::OnObjectModified.AddUObject(this,
                                                               &UNameConventionRenameAction::ResetCacheIfTableModified);
        for (const auto RowName : NameConventionsTable->GetRowNames())
        {
            const auto NameConvention = NameConventionsTable->FindRow<FNameConvention>(RowName, TEXT(""));
            // ReSharper disable once CppTooWideScopeInitStatement
            const auto ObjectType = NameConvention->ObjectType.Get();
            if (NameConvention && IsValid(ObjectType))
            {
                TArray<FNameConvention>& TypeConventions = NameConventionsCache.FindOrAdd(ObjectType);
                TypeConventions.Add(*NameConvention);
                TypeConventions.Sort();
            }
        }
        for (auto NameConventionEntry : NameConventionsCache)
        {
            LogInfo(nullptr,
                    FString::Printf(TEXT("Type %s contains %d conventions in cache"),
                                    *NameConventionEntry.Key->GetName(),
                                    NameConventionEntry.Value.Num()));
        }
    }
}
