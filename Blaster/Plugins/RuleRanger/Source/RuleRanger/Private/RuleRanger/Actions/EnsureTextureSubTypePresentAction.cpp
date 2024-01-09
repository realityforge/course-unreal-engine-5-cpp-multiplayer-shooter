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

#include "EnsureTextureSubTypePresentAction.h"
#include "Editor.h"

bool FTextureSubTypeNameConvention::DoesTextMatchAtIndex(const FString& InText, const int32 Index) const
{
    if (0 != Text.Len() && Index + Text.Len() <= InText.Len())
    {
        for (int32 i = 0; i < Text.Len(); i++)
        {
            if (InText[Index + i] != Text[i])
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

void UEnsureTextureSubTypePresentAction::ApplyRuleToTexture(URuleRangerActionContext* ActionContext,
                                                            const UTexture2D* Texture)
{
    if (const auto SubTypes = ExtractSubTypes(Texture->GetName()); SubTypes.IsEmpty())
    {
        if (bNotifyIfNameConventionMissing)
        {
            ActionContext->Error(NSLOCTEXT("RuleRanger",
                                           "UnableToParseTextureSubTypes",
                                           "Unable to parse texture subtypes. They could "
                                           "be missing or formatted incorrectly"));
        }
        else
        {
            LogInfo(Texture,
                    TEXT("Unable to parse texture subtypes. They could be missing "
                         "or formatted incorrectly. Assuming missing which is valid "
                         "when action is configured with bNotifyIfNameConventionMissing=true."));
        }
    }
    else
    {
        ApplyRuleToTextureWithSubTypes(ActionContext, Texture, SubTypes);
    }
}

void UEnsureTextureSubTypePresentAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    if (IsValid(NameConventionsTable))
    {
        RebuildNameConventionsCacheIfNecessary();

        if (!NameConventionsCache.IsEmpty())
        {
            const auto Texture = CastChecked<UTexture2D>(Object);
            ApplyRuleToTexture(ActionContext, Texture);
        }
    }
    else
    {
        LogError(Object, TEXT("Action can not run as has not specified NameConventionsTable property."));
    }
}

UClass* UEnsureTextureSubTypePresentAction::GetExpectedType()
{
    return UTexture2D::StaticClass();
}

void UEnsureTextureSubTypePresentAction::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    // ReSharper disable once CppTooWideScopeInitStatement
    const FName TableName = GET_MEMBER_NAME_CHECKED(UEnsureTextureSubTypePresentAction, NameConventionsTable);
    if (TableName == PropertyName)
    {
        ResetNameConventionsCache();
    }
    Super::PostEditChangeProperty(PropertyChangedEvent);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UEnsureTextureSubTypePresentAction::ResetCacheIfTableModified(UObject* Object)
{
    // This is called on any object edit in editor so match against conventions table and bust cache as appropriate;
    if (Object && Object == NameConventionsTable)
    {
        ResetNameConventionsCache();
    }
}

void UEnsureTextureSubTypePresentAction::ResetNameConventionsCache()
{
    LogInfo(nullptr, TEXT("Resetting the Name Conventions Cache."));

    NameConventionsCache.Empty();
    FCoreUObjectDelegates::OnObjectModified.Remove(OnObjectModifiedDelegateHandle);
    OnObjectModifiedDelegateHandle.Reset();
}

void UEnsureTextureSubTypePresentAction::RebuildNameConventionsCacheIfNecessary()
{
    if (NameConventionsCache.IsEmpty() && 0 != NameConventionsTable->GetTableData().Num())
    {
        ResetNameConventionsCache();
        // Add a callback for when ANY object is modified in the editor so that we can bust the cache
        OnObjectModifiedDelegateHandle = FCoreUObjectDelegates::OnObjectModified.AddUObject(
            this,
            &UEnsureTextureSubTypePresentAction::ResetCacheIfTableModified);
        for (const auto RowName : NameConventionsTable->GetRowNames())
        {
            const auto NameConvention = NameConventionsTable->FindRow<FTextureSubTypeNameConvention>(RowName, TEXT(""));
            NameConventionsCache.Add(*NameConvention);
            NameConventionsCache.Sort();
        }
        for (auto NameConventionEntry : NameConventionsCache)
        {
            LogInfo(nullptr, FString::Printf(TEXT("%d conventions in cache"), NameConventionsCache.Num()));
        }
    }
}

TArray<ETextureSubType> UEnsureTextureSubTypePresentAction::ExtractSubTypes(const FString& Name)
{
    TArray<ETextureSubType> SubTypes;
    int32 CurrentIndex = 0;
    if (Name.FindLastChar(TEXT('_'), CurrentIndex) && INDEX_NONE != CurrentIndex)
    {
        // Start after the last _
        CurrentIndex += 1;

        while (CurrentIndex < Name.Len())
        {
            bool bMatched = false;
            for (auto NameConvention : NameConventionsCache)
            {
                if (NameConvention.DoesTextMatchAtIndex(Name, CurrentIndex))
                {
                    SubTypes.Add(NameConvention.SubType);
                    CurrentIndex += NameConvention.Text.Len();
                    bMatched = true;
                    break;
                }
            }
            if (!bMatched)
            {
                SubTypes.Reset();
                return SubTypes;
            }
        }
        // Fall through if we have got to the end of the string and return matched SubTypes
    }
    return SubTypes;
}

void UEnsureTextureSubTypePresentAction::ApplyRuleToTextureWithSubTypes(URuleRangerActionContext* ActionContext,
                                                                        const UTexture2D* Texture,
                                                                        const TArray<ETextureSubType>& SubTypes) const
{
    check(!SubTypes.IsEmpty());
    int NumComponentsDeclared = 0;
    for (const ETextureSubType SubType : SubTypes)
    {
        switch (SubType)
        {
            case ETextureSubType::AT_Roughness:
            case ETextureSubType::AT_Opacity:
            case ETextureSubType::AT_AmbientOcclusion:
            case ETextureSubType::AT_Specular:
            case ETextureSubType::AT_Metallic:
            case ETextureSubType::AT_Height:
            case ETextureSubType::AT_FlowMap:
            case ETextureSubType::AT_Displacement:
                NumComponentsDeclared += 1;
                break;
            case ETextureSubType::AT_Normal:
                // Normals only store X and Y and recompute Z in app
                NumComponentsDeclared += 2;
                break;
            case ETextureSubType::AT_BaseColor:
            case ETextureSubType::AT_Emissive:
            case ETextureSubType::AT_LightMap:
            case ETextureSubType::AT_Mask:
                NumComponentsDeclared += 3;
                break;
            default:
            {
                const UEnum* const Enum = StaticEnum<ETextureSubType>();
                const FString Value = Enum->GetNameStringByValue(static_cast<uint8>(SubType));
                ActionContext->Error(FText::FormatNamed(
                    NSLOCTEXT("RuleRanger", "UnhandledSubType", "Unable to parse texture subtype {SubType}"),
                    TEXT("SubType"),
                    FText::FromString(Value)));
            }
            break;
        }
    }
    const float NumComponents = GPixelFormats[Texture->GetPixelFormat()].NumComponents;
    if (NumComponents != NumComponentsDeclared && NumComponents < NumComponentsDeclared)
    {
        // If NumComponents > NumComponentsDeclared then that is probably ok as most compression schemes
        // require a certain number of components per pixel to compress and Unreal just 0 fills other components
        ActionContext->Error(FText::FormatNamed(NSLOCTEXT("RuleRanger",
                                                          "MismatchingComponent",
                                                          "Pixel format specifies "
                                                          "{NumComponentsDeclared} components but "
                                                          "the subtypes declared in suffix has "
                                                          "{NumComponents} components"),
                                                TEXT("NumComponentsDeclared"),
                                                NumComponentsDeclared,
                                                TEXT("NumComponents"),
                                                NumComponents));
    }
    else
    {
        if (FTextureSubTypeUtil::DoesMetaDataMatch(Texture, SubTypes))
        {
            LogInfo(Texture,
                    TEXT("Subtypes extracted, match the component size of textures and are encoded in MetaData."));
        }
        else if (ActionContext->IsDryRun())
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("MetaDataKey"), FText::FromName(FTextureSubTypeUtil::GetMetaDataKey()));
            Arguments.Add(TEXT("MetaDataValue"), FText::FromString(FTextureSubTypeUtil::EncodeSubTypes(SubTypes)));
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "TextureSubTypeMetadataSetOmitted",
                                                          "Texture needs to set MetaData '{MetaDataKey}' "
                                                          "to '{MetaDataValue}'. Action skipped in DryRun mode"),
                                                Arguments);
            ActionContext->Warning(Message);
        }
        else if (FTextureSubTypeUtil::EncodeInMetaData(Texture, SubTypes))
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("MetaDataKey"), FText::FromName(FTextureSubTypeUtil::GetMetaDataKey()));
            Arguments.Add(TEXT("MetaDataValue"), FText::FromString(FTextureSubTypeUtil::EncodeSubTypes(SubTypes)));
            const auto Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                         "TextureSubTypeMetadataSet",
                                                         "Texture set MetaData '{MetaDataKey}' to '{MetaDataValue}'"),
                                               Arguments);
            ActionContext->Info(Message);
        }
        else
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("MetaDataKey"), FText::FromName(FTextureSubTypeUtil::GetMetaDataKey()));
            Arguments.Add(TEXT("MetaDataValue"), FText::FromString(FTextureSubTypeUtil::EncodeSubTypes(SubTypes)));
            const auto Message =
                FText::Format(NSLOCTEXT("RuleRanger",
                                        "TextureSubTypeMetadataSetFailed",
                                        "Texture failed to set MetaData '{MetaDataKey}' to '{MetaDataValue}'"),
                              Arguments);
            ActionContext->Error(Message);
        }
    }
}
