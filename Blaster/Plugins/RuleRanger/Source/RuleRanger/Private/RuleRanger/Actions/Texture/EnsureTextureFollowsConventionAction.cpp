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

#include "EnsureTextureFollowsConventionAction.h"
#include "RuleRanger/RuleRangerUtilities.h"
#include "RuleRangerConfig.h"
#include "Subsystems/EditorAssetSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureTextureFollowsConventionAction)

static FName NAME_RuleRanger_Variant("RuleRanger.Variant");

void UEnsureTextureFollowsConventionAction::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    if ((GET_MEMBER_NAME_CHECKED(UEnsureTextureFollowsConventionAction, ConventionsTables)) == PropertyName)
    {
        ResetConventionsCache();
    }
    else if ((GET_MEMBER_NAME_CHECKED(URuleRangerConfig, DataTables)) == PropertyName)
    {
        ResetConventionsCache();
    }
    Super::PostEditChangeProperty(PropertyChangedEvent);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UEnsureTextureFollowsConventionAction::ResetCacheIfTableModified(UObject* Object)
{
    if (Object)
    {
        // This is called on any object edited in the editor so cache and bust cache as appropriate;
        if (ConventionsTables.Contains(Object) || ConfigConventionsTables.Contains(Object))
        {
            LogInfo(nullptr,
                    FString::Printf(TEXT("ResetCacheIfTableModified invoked for %s and caused a reset"),
                                    *Object->GetName()));
            ResetConventionsCache();
        }
    }
}

void UEnsureTextureFollowsConventionAction::ResetConventionsCache()
{
    LogInfo(nullptr, TEXT("Resetting the Conventions Cache"));

    ConventionsCache.Empty();
    FCoreUObjectDelegates::OnObjectModified.Remove(OnObjectModifiedDelegateHandle);
    OnObjectModifiedDelegateHandle.Reset();
}

void UEnsureTextureFollowsConventionAction::RebuildConventionsCacheIfNecessary()
{
    check(!ConventionsTables.IsEmpty() || !ConfigConventionsTables.IsEmpty());

    TArray<UDataTable*> Tables;
    Tables.Append(ConventionsTables);
    Tables.Append(ConfigConventionsTables);

    bool bTableDataPresent = false;
    for (const auto& ConventionsTable : Tables)
    {
        if (IsValid(ConventionsTable) && 0 != ConventionsTable->GetTableData().Num())
        {
            bTableDataPresent = true;
            break;
        }
    }

    if (ConventionsCache.IsEmpty() && bTableDataPresent)
    {
        ResetConventionsCache();
        // Add a callback for when ANY object is modified in the editor so that we can bust the cache
        OnObjectModifiedDelegateHandle = FCoreUObjectDelegates::OnObjectModified.AddUObject(
            this,
            &UEnsureTextureFollowsConventionAction::ResetCacheIfTableModified);
        for (const auto& ConventionsTable : Tables)
        {
            if (IsValid(ConventionsTable))
            {
                for (const auto& RowName : ConventionsTable->GetRowNames())
                {
                    if (const auto& Convention =
                            ConventionsTable->FindRow<FRuleRangerTextureConvention>(RowName, TEXT("")))
                    {
                        ConventionsCache.Emplace(RowName, *Convention);
                    }
                }
            }
        }
        LogInfo(nullptr,
                FString::Printf(TEXT("RebuildConventionsCacheIfNecessary rebuilt cache and added %d conventions"),
                                ConventionsCache.Num()));
    }
}

void UEnsureTextureFollowsConventionAction::CheckPowerOfTwo(URuleRangerActionContext* ActionContext,
                                                            const UTexture2D* const Texture) const
{
    const int32 SizeX = Texture->GetSizeX();
    const int32 SizeY = Texture->GetSizeY();

    const bool bInvalidX = (0 != (SizeX & SizeX - 1));
    const bool bInvalidY = (0 != (SizeY & SizeY - 1));

    if (bInvalidX || bInvalidY)
    {
        FFormatNamedArguments Arguments;
        Arguments.Add(TEXT("X"), FText::FromString(FString::FromInt(SizeX)));
        Arguments.Add(TEXT("Y"), FText::FromString(FString::FromInt(SizeY)));

        if (bInvalidX && bInvalidY)
        {
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "EnsureTextureFollowsConventionAction_Pow2FailXY",
                                                          "Texture has dimensions {X}x{Y} and neither width nor"
                                                          " height is a power of two. Fix both dimensions"),
                                                Arguments);
            ActionContext->Error(Message);
        }
        else if (bInvalidX)
        {
            const FText Message = FText::Format(
                NSLOCTEXT("RuleRanger",
                          "EnsureTextureFollowsConventionAction_Pow2FailX",
                          "Texture has dimensions {X}x{Y} and width is not a power of two. Fix the width dimension"),
                Arguments);
            ActionContext->Error(Message);
        }
        else
        {
            const FText Message = FText::Format(
                NSLOCTEXT("RuleRanger",
                          "EnsureTextureFollowsConventionAction_Pow2FailY",
                          "Texture has dimensions {X}x{Y} and height is not a power of two. Fix the height dimension"),
                Arguments);
            ActionContext->Error(Message);
        }
    }
    else
    {
        LogInfo(Texture, TEXT("Texture dimensions are a power of two. No Action required."));
    }
}

void UEnsureTextureFollowsConventionAction::CheckDivisibleConstraint(URuleRangerActionContext* ActionContext,
                                                                     const ETextureResolutionConstraint Constraint,
                                                                     const UTexture2D* const Texture) const
{
    const int Divisor = ETextureResolutionConstraint::DivisibleByFour == Constraint ? 4
        : ETextureResolutionConstraint::DivisibleByEight == Constraint              ? 8
                                                                                    : 12;
    const int32 SizeX = Texture->GetSizeX();
    const int32 SizeY = Texture->GetSizeY();

    const bool bInvalidX = 0 != (SizeX % Divisor);
    const bool bInvalidY = 0 != (SizeY % Divisor);

    if (bInvalidX || bInvalidY)
    {
        FFormatNamedArguments Arguments;
        Arguments.Add(TEXT("X"), FText::FromString(FString::FromInt(SizeX)));
        Arguments.Add(TEXT("Y"), FText::FromString(FString::FromInt(SizeY)));
        Arguments.Add(TEXT("Divisor"), FText::FromString(FString::FromInt(Divisor)));

        if (bInvalidX && bInvalidY)
        {
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "EnsureTextureFollowsConventionAction_DivFailXY",
                                                          "Texture has dimensions {X}x{Y} and neither width nor"
                                                          " height is divisible by {Divisor}. Fix both dimensions"),
                                                Arguments);
            ActionContext->Error(Message);
        }
        else if (bInvalidX)
        {
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "EnsureTextureFollowsConventionAction_DivFailX",
                                                          "Texture has dimensions {X}x{Y} and width is not "
                                                          "divisible by {Divisor}. Fix the width dimension"),
                                                Arguments);
            ActionContext->Error(Message);
        }
        else
        {
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "EnsureTextureFollowsConventionAction_DivFailY",
                                                          "Texture has dimensions {X}x{Y} and height is not "
                                                          "divisible by {Divisor}. Fix the height dimension"),
                                                Arguments);
            ActionContext->Error(Message);
        }
    }
    else
    {
        LogInfo(Texture, FString::Printf(TEXT("Texture dimensions are divisible by %d. No Action required."), Divisor));
    }
}

void UEnsureTextureFollowsConventionAction::PerformTextureResolutionConstraintCheck(
    URuleRangerActionContext* ActionContext,
    const UTexture2D* const Texture,
    const FRuleRangerTextureConvention* const Convention) const
{
    if (ETextureResolutionConstraint::PowerOfTwo == Convention->TextureResolutionConstraint)
    {
        CheckPowerOfTwo(ActionContext, Texture);
    }
    else if (ETextureResolutionConstraint::Auto != Convention->TextureResolutionConstraint)
    {
        CheckDivisibleConstraint(ActionContext, Convention->TextureResolutionConstraint, Texture);
    }
}

void UEnsureTextureFollowsConventionAction::PerformTextureGroupCheck(
    URuleRangerActionContext* ActionContext,
    UTexture2D* const Texture,
    FRuleRangerTextureConvention* const Convention) const
{
    if (!Convention->TextureGroups.IsEmpty() && !Convention->TextureGroups.Contains(Texture->LODGroup))
    {
        const UEnum* Enum = StaticEnum<TextureGroup>();
        if (bApplyFix && Convention->TextureGroups.Num() > 0)
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("OriginalGroup"), Enum->GetDisplayNameTextByValue(Texture->CompressionSettings));
            Arguments.Add(TEXT("NewGroup"), Enum->GetDisplayNameTextByValue(Convention->TextureGroups[0]));
            if (ActionContext->IsDryRun())
            {
                const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                              "TextureCompressionChangeOmitted",
                                                              "Texture should have the TextureGroup changed from "
                                                              "'{OriginalGroup}' to '{NewGroup}'. "
                                                              "Action skipped in DryRun mode"),
                                                    Arguments);
                ActionContext->Warning(Message);
            }
            else
            {
                const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                              "TextureGroupChanged",
                                                              "Texture changed the TextureGroup from "
                                                              "'{OriginalGroup}' to '{NewGroup}'"),
                                                    Arguments);
                ActionContext->Info(Message);

                Texture->LODGroup = Convention->TextureGroups[0];

                ensure(Texture->MarkPackageDirty());
                ensure(Texture->GetOuter()->MarkPackageDirty());
            }
        }
        else
        {
            FString ValidGroups{ "" };
            for (const auto& Group : Convention->TextureGroups)
            {
                if (ValidGroups.Len() > 0)
                {
                    ValidGroups.Append(", ");
                }
                ValidGroups.Append(Enum->GetDisplayNameTextByValue(Group).ToString());
            }
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("ActualGroup"), Enum->GetDisplayNameTextByValue(Texture->LODGroup));
            Arguments.Add(TEXT("ValidGroups"), FText::FromString(ValidGroups));
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "EnsureTextureGroupValidAction_Fail",
                                                          "Texture has the TextureGroup '{ActualGroup}' that is not "
                                                          "one of the valid TextureGroups: '{ValidGroups}'"),
                                                Arguments);
            ActionContext->Error(Message);
        }
    }
    else
    {
        LogInfo(Texture, TEXT("Texture has a valid TextureGroup. No Action required."));
    }
}

void UEnsureTextureFollowsConventionAction::PerformColorSpaceCheck(
    URuleRangerActionContext* ActionContext,
    UTexture2D* const Texture,
    const FRuleRangerTextureConvention* const Convention) const
{
    if (ERuleRangerTextureColorSpace::Auto != Convention->ColorSpace)
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        const bool bSRGB = Convention->ColorSpace == ERuleRangerTextureColorSpace::SRGB;
        if (Texture->SRGB != bSRGB)
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("Original"), FText::FromString(bSRGB ? TEXT("false") : TEXT("true")));
            Arguments.Add(TEXT("New"), FText::FromString(bSRGB ? TEXT("true") : TEXT("false")));
            if (ActionContext->IsDryRun())
            {
                const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                              "TextureSRGBChangeOmitted",
                                                              "Texture should have sRGB changed from "
                                                              "'{Original}' to '{New}'. "
                                                              "Action skipped in DryRun mode"),
                                                    Arguments);
                ActionContext->Warning(Message);
            }
            else
            {
                const FText Message = FText::Format(
                    NSLOCTEXT("RuleRanger", "TextureSRGBChanged", "Texture changed sRGB from '{Original}' to '{New}'"),
                    Arguments);
                ActionContext->Info(Message);

                Texture->SRGB = bSRGB;

                ensure(Texture->MarkPackageDirty());
                ensure(Texture->GetOuter()->MarkPackageDirty());
            }
        }
        else
        {
            LogInfo(Texture, TEXT("Texture has a valid sRGB. No Action required."));
        }
    }
}

void UEnsureTextureFollowsConventionAction::PerformMipGenSettingsCheck(
    URuleRangerActionContext* ActionContext,
    UTexture2D* const Texture,
    const FRuleRangerTextureConvention* const Convention) const
{
    if (!Convention->TextureMipGenSettings.IsEmpty()
        && !Convention->TextureMipGenSettings.Contains(Texture->MipGenSettings))
    {
        const UEnum* Enum = StaticEnum<TextureMipGenSettings>();
        if (bApplyFix && Convention->TextureMipGenSettings.Num() > 0)
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("OriginalSetting"), Enum->GetDisplayNameTextByValue(Texture->MipGenSettings));
            Arguments.Add(TEXT("NewSetting"), Enum->GetDisplayNameTextByValue(Convention->TextureMipGenSettings[0]));
            if (ActionContext->IsDryRun())
            {
                const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                              "TextureMipGenChangeOmitted",
                                                              "Texture should have the MipGen setting changed from "
                                                              "'{OriginalSetting}' to '{NewSetting}'. "
                                                              "Action skipped in DryRun mode"),
                                                    Arguments);
                ActionContext->Warning(Message);
            }
            else
            {
                const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                              "TextureMipGenChanged",
                                                              "Texture changed the compression setting from "
                                                              "'{OriginalSetting}' to '{NewSetting}'"),
                                                    Arguments);
                ActionContext->Info(Message);

                Texture->MipGenSettings = Convention->TextureMipGenSettings[0];

                ensure(Texture->MarkPackageDirty());
                ensure(Texture->GetOuter()->MarkPackageDirty());
            }
        }
        else
        {
            FString ValidSettings{ "" };
            for (const auto& Setting : Convention->TextureMipGenSettings)
            {
                if (ValidSettings.Len() > 0)
                {
                    ValidSettings.Append(", ");
                }
                ValidSettings.Append(Enum->GetDisplayNameTextByValue(Setting).ToString());
            }
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("ActualSettings"), Enum->GetDisplayNameTextByValue(Texture->MipGenSettings));
            Arguments.Add(TEXT("ValidSettings"), FText::FromString(ValidSettings));
            const FText Message =
                FText::Format(NSLOCTEXT("RuleRanger",
                                        "EnsureTextureMipGenValidAction_Fail",
                                        "Texture has the MipGen setting '{ActualSettings}' that is not "
                                        "one of the valid MipGen settings '{ValidSettings}' and the "
                                        "ApplyFix action parameter has not been specified"),
                              Arguments);
            ActionContext->Error(Message);
        }
    }
    else
    {
        LogInfo(Texture, TEXT("Texture has a valid MipGen Setting. No Action required."));
    }
}

void UEnsureTextureFollowsConventionAction::PerformTextureCompressionCheck(
    URuleRangerActionContext* ActionContext,
    UTexture2D* const Texture,
    FRuleRangerTextureConvention* const Convention) const
{
    if (!Convention->TextureCompressionSettings.IsEmpty()
        && !Convention->TextureCompressionSettings.Contains(Texture->CompressionSettings))
    {
        const UEnum* Enum = StaticEnum<TextureCompressionSettings>();
        if (bApplyFix && Convention->TextureCompressionSettings.Num() > 0)
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("OriginalSetting"), Enum->GetDisplayNameTextByValue(Texture->CompressionSettings));
            Arguments.Add(TEXT("NewSetting"),
                          Enum->GetDisplayNameTextByValue(Convention->TextureCompressionSettings[0]));
            if (ActionContext->IsDryRun())
            {
                const FText Message =
                    FText::Format(NSLOCTEXT("RuleRanger",
                                            "TextureCompressionChangeOmitted",
                                            "Texture should have the compression setting changed from "
                                            "'{OriginalSetting}' to '{NewSetting}'. "
                                            "Action skipped in DryRun mode"),
                                  Arguments);
                ActionContext->Warning(Message);
            }
            else
            {
                const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                              "TextureCompressionChanged",
                                                              "Texture changed the compression setting from "
                                                              "'{OriginalSetting}' to '{NewSetting}'"),
                                                    Arguments);
                ActionContext->Info(Message);

                Texture->CompressionSettings = Convention->TextureCompressionSettings[0];

                ensure(Texture->MarkPackageDirty());
                ensure(Texture->GetOuter()->MarkPackageDirty());
            }
        }
        else
        {
            FString ValidSettings{ "" };
            for (const auto& Setting : Convention->TextureCompressionSettings)
            {
                if (ValidSettings.Len() > 0)
                {
                    ValidSettings.Append(", ");
                }
                ValidSettings.Append(Enum->GetDisplayNameTextByValue(Setting).ToString());
            }
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("ActualSettings"), Enum->GetDisplayNameTextByValue(Texture->CompressionSettings));
            Arguments.Add(TEXT("ValidSettings"), FText::FromString(ValidSettings));
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "EnsureTextureCompressionValidAction_Fail",
                                                          "Texture has the compression '{ActualSettings}' that is not "
                                                          "one of the valid Texture Compressions '{ValidSettings}' and "
                                                          "ApplyFix action parameter has not been specified"),
                                                Arguments);
            ActionContext->Error(Message);
        }
    }
    else
    {
        LogInfo(Texture, TEXT("Texture has a valid Texture Compression Setting. No Action required."));
    }
}

FName UEnsureTextureFollowsConventionAction::FindVariantBySuffix(const UTexture2D* Texture)
{
    const FString OriginalName{ Texture->GetName() };

    TArray<FName> Keys;
    ConventionsCache.GetKeys(Keys);
    for (const auto Key : Keys)
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto& TextureConvention = ConventionsCache.FindChecked(Key);
        if (!TextureConvention.Suffix.IsEmpty()
            && OriginalName.EndsWith(TextureConvention.Suffix, ESearchCase::CaseSensitive))
        {
            return Key;
        }
    }
    return NAME_None;
}

void UEnsureTextureFollowsConventionAction::RebuildConfigConventionsTables(
    const URuleRangerActionContext* ActionContext)
{
    ConfigConventionsTables.Reset();
    for (const auto DataTable : ActionContext->GetOwnerConfig()->DataTables)
    {
        if (IsValid(DataTable))
        {
            if (FRuleRangerTextureConvention::StaticStruct() == DataTable->RowStruct)
            {
                LogInfo(
                    nullptr,
                    FString::Printf(TEXT("Adding DataTable '%s' registered in Config %s to set of conventions applied"),
                                    *DataTable.GetName(),
                                    *ActionContext->GetOwnerConfig()->GetName()));
                ConfigConventionsTables.Add(DataTable);
            }
        }
    }
}

void UEnsureTextureFollowsConventionAction::PerformSetVariantMetaDataCheck(URuleRangerActionContext* ActionContext,
                                                                           UTexture2D* Texture,
                                                                           const FName& ConventionKey) const
{
    const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
    // ReSharper disable once CppTooWideScopeInitStatement
    const FString ExistingValue = Subsystem->GetMetadataTag(Texture, NAME_RuleRanger_Variant);
    if (ExistingValue.Equals(ConventionKey.ToString()))
    {
        LogInfo(Texture,
                FString::Printf(TEXT("MetaDataTag %s=%s already exists on Object. No action required"),
                                *NAME_RuleRanger_Variant.ToString(),
                                *ConventionKey.ToString()));
    }
    else
    {
        if (ActionContext->IsDryRun())
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("Key"), FText::FromString(NAME_RuleRanger_Variant.ToString()));
            Arguments.Add(TEXT("Value"), FText::FromString(ConventionKey.ToString()));
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "MetaDataTagAddOmitted",
                                                          "MetaData tag {Key}={Value} is not present. This tag would "
                                                          "be added if RuleRanger was not in DryRun mode"),
                                                Arguments);

            ActionContext->Error(Message);
        }
        else
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("Key"), FText::FromString(NAME_RuleRanger_Variant.ToString()));
            Arguments.Add(TEXT("Value"), FText::FromString(ConventionKey.ToString()));
            const FText Message = FText::Format(
                NSLOCTEXT("RuleRanger", "SetMetaDataTag", "MetaData tag {Key}={Value} is not present. Adding tag."),
                Arguments);

            ActionContext->Info(Message);
            Subsystem->SetMetadataTag(Texture, NAME_RuleRanger_Variant, ConventionKey.ToString());
            // This should not be called during loads of the object, so neither of these functions should
            // return false
            ensure(Texture->MarkPackageDirty());
            ensure(Texture->GetOuter()->MarkPackageDirty());
        }
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void UEnsureTextureFollowsConventionAction::PerformNameSuffixCheck(URuleRangerActionContext* ActionContext,
                                                                   UTexture2D* const Texture,
                                                                   const FRuleRangerTextureConvention* const Convention)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const FString OriginalName{ Texture->GetName() };
    if (!Convention->Suffix.IsEmpty() && !OriginalName.EndsWith(Convention->Suffix, ESearchCase::CaseSensitive))
    {
        const FString NewName{ FString::Printf(TEXT("%s%s"), *OriginalName, *Convention->Suffix) };
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

            if (!FRuleRangerUtilities::RenameAsset(Texture, NewName))
            {
                const auto InMessage = FText::Format(
                    NSLOCTEXT("RuleRanger", "ObjectRenameFailed", "Attempt to rename object '{0}' to '{1}' failed."),
                    FText::FromString(OriginalName),
                    FText::FromString(NewName));
                ActionContext->Error(InMessage);
            }
        }
    }
}

void UEnsureTextureFollowsConventionAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                 UObject* Object)
{
    RebuildConfigConventionsTables(ActionContext);

    if (!ConventionsTables.IsEmpty() || !ConfigConventionsTables.IsEmpty())
    {
        RebuildConventionsCacheIfNecessary();

        const auto Texture = CastChecked<UTexture2D>(Object);
        const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
        const auto DeclaredVariant =
            Subsystem ? FName(Subsystem->GetMetadataTag(Object, NAME_RuleRanger_Variant)) : NAME_None;
        const auto Variant = NAME_None == DeclaredVariant ? FindVariantBySuffix(Texture) : DeclaredVariant;

        if (const auto Convention = ConventionsCache.Find(Variant))
        {
            PerformSetVariantMetaDataCheck(ActionContext, Texture, Variant);
            PerformNameSuffixCheck(ActionContext, Texture, Convention);
            PerformTextureCompressionCheck(ActionContext, Texture, Convention);
            PerformColorSpaceCheck(ActionContext, Texture, Convention);
            PerformTextureGroupCheck(ActionContext, Texture, Convention);
            PerformTextureResolutionConstraintCheck(ActionContext, Texture, Convention);
            PerformMipGenSettingsCheck(ActionContext, Texture, Convention);
        }
        else
        {
            LogInfo(
                Object,
                FString::Printf(TEXT("Object with variant '%s' has no associated conventions"), *Variant.ToString()));
        }
    }
}
