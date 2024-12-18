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
#include "EnsureNameFollowsConventionAction.h"
#include "Editor.h"
#include "RuleRanger/RuleRangerUtilities.h"
#include "Subsystems/EditorAssetSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureNameFollowsConventionAction)

bool UEnsureNameFollowsConventionAction::FindMatchingNameConvention(URuleRangerActionContext* ActionContext,
                                                                    const UObject* Object,
                                                                    const TArray<UClass*>& Classes,
                                                                    const FString& Variant,
                                                                    FNameConvention& MatchingConvention) const
{
    for (const auto& Class : Classes)
    {
        if (const auto Conventions = ConventionsCache.Find(Class))
        {
            for (int32 Index = 0; Index < Conventions->Num(); ++Index)
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                const auto Convention = Conventions->GetData()[Index];
                if (Convention.Variant.Equals(Variant) || Convention.Variant.Equals(NameConvention_DefaultVariant))
                {
                    LogInfo(Object,
                            FString::Printf(TEXT("Located naming convention rule for "
                                                 "(Class %s, Variant '%s') (Prefix = '%s', Suffix = '%s')"),
                                            *Object->GetClass()->GetName(),
                                            *Variant,
                                            *Convention.Prefix,
                                            *Convention.Suffix));
                    MatchingConvention = Convention;
                    return true;
                }
            }
        }
    }
    const auto Message = FString::Printf(TEXT("Unable to locate naming convention rule for (Class %s, Variant '%s')"),
                                         *Object->GetClass()->GetName(),
                                         *Variant);
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto OutermostObject = Object->GetOutermostObject();
    if (bNotifyIfNameConventionMissing && OutermostObject == Object)
    {
        // Only attempt to apply naming conventions to outermost packages
        ActionContext->Warning(FText::FromString(Message));
    }
    else
    {
        LogInfo(Object, Message);
    }
    return false;
}

void UEnsureNameFollowsConventionAction::RebuildCachesIfNecessary()
{
    RebuildDeprecatedConventionCacheIfNecessary();
    RebuildConventionCacheIfNecessary();

    if (!OnObjectModifiedDelegateHandle.IsValid())
    {
        // Add a callback for when ANY object is modified in the editor so that we can bust the cache
        OnObjectModifiedDelegateHandle = FCoreUObjectDelegates::OnObjectModified.AddUObject(
            this,
            &UEnsureNameFollowsConventionAction::ResetCachesIfTablesModified);
    }
}

void UEnsureNameFollowsConventionAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    static FName NAME_RuleRanger_Variant("RuleRanger.Variant");

    RebuildCachesIfNecessary();

    const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
    const auto Variant = Subsystem ? Subsystem->GetMetadataTag(Object, NAME_RuleRanger_Variant) : TEXT("");

    const FString OriginalName{ Object->GetName() };

    FString NewName{ OriginalName };

    TArray<UClass*> Classes;
    FRuleRangerUtilities::CollectTypeHierarchy(Object, Classes);

    if (Classes.Contains(UObjectRedirector::StaticClass()))
    {
        LogInfo(Object, TEXT("Object is an ObjectRedirector and can not be renamed. No action required."));
        return;
    }
    else if (Classes.Contains(AActor::StaticClass()))
    {
        LogInfo(Object,
                TEXT("Object is an Actor and actor naming is not managed the "
                     "EnsureNameFollowsConventionAction. No action required."));
        return;
    }

    // Find the naming convention that matches the object closely.
    // i.e. The most specific type, with a matching variant.
    FNameConvention MatchingNameConvention;
    const bool bMatched = FindMatchingNameConvention(ActionContext, Object, Classes, Variant, MatchingNameConvention);
    const FString ExpectedPrefix{ bMatched ? MatchingNameConvention.Prefix : FString("") };
    const FString ExpectedSuffix{ bMatched ? MatchingNameConvention.Suffix : FString("") };

    for (const auto& Conventions : DeprecatedConventionsCache)
    {
        if (Classes.Contains(Conventions.Key.Get()))
        {
            for (auto& NameConvention : Conventions.Value)
            {
                if ((NameConvention.Prefix.IsEmpty()
                     || NewName.StartsWith(NameConvention.Prefix, ESearchCase::CaseSensitive))
                    && (NameConvention.Suffix.IsEmpty()
                        || NewName.EndsWith(NameConvention.Suffix, ESearchCase::CaseSensitive)))
                {
                    const FString PreRenameName{ NewName };
                    NewName = NewName.RightChop(NameConvention.Prefix.Len()).LeftChop(NameConvention.Suffix.Len());
                    LogInfo(
                        Object,
                        FString::Printf(
                            TEXT(
                                "Cleaning up deprecated name convention (Class %s, Prefix '%s', Suffix '%s'). Input name: %s, Output name: %s"),
                            *NameConvention.ObjectType->GetName(),
                            *NameConvention.Prefix,
                            *NameConvention.Suffix,
                            *PreRenameName,
                            *NewName));
                }
            }
        }
    }

    for (const TPair<TObjectPtr<UClass>, TArray<FNameConvention>>& NameConventions : ConventionsCache)
    {
        const auto Type = NameConventions.Key.Get();
        const bool bMatchesTypeHierarchy = Classes.Contains(Type);
        for (auto& NameConvention : NameConventions.Value)
        {
            // First, we process all the rules that do not match:
            // - We remove a prefix if it is different from the matching convention prefix, and
            //   the prefix matches another convention's prefix in the convention list.
            // - We remove a suffix if it is different from the matching convention suffix, and
            //   is for a convention that matches the type.
            //
            // This assumes prefixes are primary determinants of type while suffixes are usually
            // discriminators of subtypes. i.e. `M_` indicates material type, `_BC` indicates the
            // "Base Color" material "subtype". The above rules help reinforce this.
            if (!bMatched || MatchingNameConvention != NameConvention)
            {
                if (!NameConvention.Prefix.IsEmpty()
                    && (ExpectedPrefix.IsEmpty() || !ExpectedPrefix.StartsWith(NameConvention.Prefix))
                    && NewName.StartsWith(NameConvention.Prefix, ESearchCase::CaseSensitive))
                {
                    LogInfo(Object,
                            FString::Printf(TEXT("Removing prefix '%s' as the name convention "
                                                 "(Class %s, Variant '%s') claimed that prefix"),
                                            *NameConvention.Prefix,
                                            *NameConvention.ObjectType->GetName(),
                                            *NameConvention.Variant));
                    NewName = NewName.RightChop(NameConvention.Prefix.Len());
                }

                if (bMatchesTypeHierarchy && !NameConvention.Suffix.IsEmpty()
                    && (ExpectedSuffix.IsEmpty() || !ExpectedSuffix.StartsWith(NameConvention.Suffix))
                    && NewName.EndsWith(NameConvention.Suffix, ESearchCase::CaseSensitive))
                {
                    LogInfo(Object,
                            FString::Printf(TEXT("Removing suffix '%s' as the name convention "
                                                 "(Class %s, Variant '%s') claimed that suffix"),
                                            *NameConvention.Suffix,
                                            *NameConvention.ObjectType->GetName(),
                                            *NameConvention.Variant));
                    NewName = NewName.LeftChop(NameConvention.Suffix.Len());
                }
            }
        }

        if (bMatched)
        {
            if (!MatchingNameConvention.Prefix.IsEmpty()
                && !NewName.StartsWith(MatchingNameConvention.Prefix, ESearchCase::CaseSensitive))
            {
                NewName.InsertAt(0, MatchingNameConvention.Prefix);
            }
            if (!MatchingNameConvention.Suffix.IsEmpty()
                && !NewName.EndsWith(MatchingNameConvention.Suffix, ESearchCase::CaseSensitive))
            {
                NewName.Append(MatchingNameConvention.Suffix);
            }
        }
    }

    if (NewName.Equals(OriginalName, ESearchCase::CaseSensitive))
    {
        if (bMatched)
        {
            LogInfo(Object, TEXT("Object matches naming convention. No action required."));
        }
        else
        {
            LogInfo(Object,
                    TEXT("Object matches no naming convention and does use "
                         "reserved prefixes or suffixes or deprecated naming conventions."
                         " No action required."));
        }
    }
    else
    {
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
        else if (!Object->IsAsset())
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("OriginalName"), FText::FromString(OriginalName));
            Arguments.Add(TEXT("NewName"), FText::FromString(NewName));
            const auto Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                         "ObjectRenameOmittedForNonAsset",
                                                         "Object needs to be renamed from '{OriginalName}' "
                                                         "to '{NewName}'. Rename can not be automated "
                                                         "as object is not an asset"),
                                               Arguments);

            ActionContext->Error(Message);
        }
        else
        {
            if (!FRuleRangerUtilities::RenameAsset(Object, NewName))
            {
                const auto InMessage = FText::Format(
                    NSLOCTEXT("RuleRanger", "ObjectRenameFailed", "Attempt to rename object '{0}' to '{1}' failed."),
                    FText::FromString(OriginalName),
                    FText::FromString(NewName));
                ActionContext->Error(InMessage);
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
            }
        }
    }
}

void UEnsureNameFollowsConventionAction::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    if ((GET_MEMBER_NAME_CHECKED(UEnsureNameFollowsConventionAction, DeprecatedConventionsTables)) == PropertyName
        || (GET_MEMBER_NAME_CHECKED(UEnsureNameFollowsConventionAction, NameConventionsTables)) == PropertyName)
    {
        ResetCaches();
    }
    Super::PostEditChangeProperty(PropertyChangedEvent);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UEnsureNameFollowsConventionAction::ResetCachesIfTablesModified(UObject* Object)
{
    // This is called on any object edit in editor so match against convention tables and bust cache as appropriate;
    if (Object && (DeprecatedConventionsTables.Contains(Object) || NameConventionsTables.Contains(Object)))
    {
        ResetCaches();
    }
}

void UEnsureNameFollowsConventionAction::ResetCaches()
{
    LogInfo(nullptr, TEXT("Resetting the Name Convention Caches"));

    DeprecatedConventionsCache.Empty();
    ConventionsCache.Empty();
    FCoreUObjectDelegates::OnObjectModified.Remove(OnObjectModifiedDelegateHandle);
    OnObjectModifiedDelegateHandle.Reset();
}

void UEnsureNameFollowsConventionAction::RebuildConventionCacheIfNecessary()
{
    if (NameConventionsTables.IsEmpty() && !ConventionsCache.IsEmpty())
    {
        ConventionsCache.Reset();
    }

    bool bTableDataPresent = false;
    for (const auto& NameConventionsTable : NameConventionsTables)
    {
        if (NameConventionsTable && 0 != NameConventionsTable->GetTableData().Num())
        {
            bTableDataPresent = true;
            break;
        }
    }
    if (ConventionsCache.IsEmpty() && bTableDataPresent)
    {
        for (const auto& NameConventionsTable : NameConventionsTables)
        {
            if (IsValid(NameConventionsTable))
            {
                for (const auto& RowName : NameConventionsTable->GetRowNames())
                {
                    const auto& Convention = NameConventionsTable->FindRow<FNameConvention>(RowName, TEXT(""));
                    // ReSharper disable once CppTooWideScopeInitStatement
                    const auto& ObjectType = Convention->ObjectType.Get();
                    if (Convention && IsValid(ObjectType))
                    {
                        TArray<FNameConvention>& TypeConventions = ConventionsCache.FindOrAdd(ObjectType);
                        TypeConventions.Add(*Convention);
                        TypeConventions.Sort();
                    }
                }
            }
        }
        for (auto Entry : ConventionsCache)
        {
            LogInfo(nullptr,
                    FString::Printf(TEXT("Type %s contains %d conventions in cache"),
                                    *Entry.Key->GetName(),
                                    Entry.Value.Num()));
        }
    }
}

void UEnsureNameFollowsConventionAction::RebuildDeprecatedConventionCacheIfNecessary()
{
    if (DeprecatedConventionsTables.IsEmpty() && !DeprecatedConventionsCache.IsEmpty())
    {
        DeprecatedConventionsCache.Reset();
    }

    bool bTableDataPresent = false;
    for (const auto& Table : DeprecatedConventionsTables)
    {
        if (Table && 0 != Table->GetTableData().Num())
        {
            bTableDataPresent = true;
            break;
        }
    }
    if (DeprecatedConventionsCache.IsEmpty() && bTableDataPresent)
    {
        for (const auto& ConventionsTable : DeprecatedConventionsTables)
        {
            if (IsValid(ConventionsTable))
            {
                for (const auto& RowName : ConventionsTable->GetRowNames())
                {
                    const auto& Convention = ConventionsTable->FindRow<FDeprecatedNameConvention>(RowName, TEXT(""));
                    // ReSharper disable once CppTooWideScopeInitStatement
                    const auto& ObjectType = Convention->ObjectType.Get();
                    if (Convention && IsValid(ObjectType))
                    {
                        DeprecatedConventionsCache.FindOrAdd(ObjectType).Add(*Convention);
                    }
                }
            }
        }
        for (auto Entry : DeprecatedConventionsCache)
        {
            LogInfo(nullptr,
                    FString::Printf(TEXT("Type %s contains %d deprecated conventions in cache"),
                                    *Entry.Key->GetName(),
                                    Entry.Value.Num()));
        }
    }
}
