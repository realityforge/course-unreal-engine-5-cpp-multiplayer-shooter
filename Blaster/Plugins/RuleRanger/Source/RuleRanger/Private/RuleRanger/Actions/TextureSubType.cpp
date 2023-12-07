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

#include "TextureSubType.h"

const FName FTextureSubTypeUtil::MetaDataKey{ TEXT("TextureSubTypes") };

TArray<ETextureSubType> FTextureSubTypeUtil::ExtractFromMetaData(const UObject* Object)
{
    TArray<ETextureSubType> SubTypes;
    if (IsValid(Object))
    {
        const UEnum* Enum = StaticEnum<ETextureSubType>();
        const int32 NumEnums = Enum->NumEnums();
        const FString Value = Object->GetPackage()->GetMetaData()->GetValue(Object, MetaDataKey);
        TArray<FString> Out;
        Value.ParseIntoArray(Out, TEXT(","), true);
        for (auto& Part : Out)
        {
            bool bFound = false;
            for (int32 i = 0; i < NumEnums; i++)
            {
                if (Enum->GetDisplayNameTextByIndex(i).ToString().Equals(Part))
                {
                    const int64 EnumValue = Enum->GetValueByIndex(i);
                    SubTypes.Add(static_cast<ETextureSubType>(EnumValue));
                    bFound = true;
                    break;
                }
            }

            if (!bFound)
            {
                SubTypes.Reset();
                return SubTypes;
            }
        }
    }
    return SubTypes;
}

bool FTextureSubTypeUtil::DoesMetaDataMatch(const UObject* Object, const TArray<ETextureSubType>& SubTypes)
{
    if (IsValid(Object))
    {
        const FString Actual = Object->GetPackage()->GetMetaData()->GetValue(Object, MetaDataKey);
        const FString Expected = EncodeSubTypes(SubTypes);
        return Expected.Equals(Actual);
    }
    else
    {
        return false;
    }
}

bool FTextureSubTypeUtil::EncodeInMetaData(const UObject* Object, const TArray<ETextureSubType>& SubTypes)
{
    if (IsValid(Object))
    {
        UMetaData* MetaData = Object->GetPackage()->GetMetaData();
        if (SubTypes.IsEmpty())
        {
            if (MetaData->HasValue(Object, MetaDataKey))
            {
                MetaData->RemoveValue(Object, MetaDataKey);
                ensure(Object->MarkPackageDirty());
                ensure(Object->GetOuter()->MarkPackageDirty());
            }
            return true;
        }
        else
        {
            const FString EncodedValue = EncodeSubTypes(SubTypes);
            if (EncodedValue.IsEmpty())
            {
                return false;
            }
            else
            {
                if (const FString Existing = MetaData->GetValue(Object, MetaDataKey); !EncodedValue.Equals(Existing))
                {
                    MetaData->SetValue(Object, MetaDataKey, *EncodedValue);
                    ensure(Object->MarkPackageDirty());
                    ensure(Object->GetOuter()->MarkPackageDirty());
                }
                return true;
            }
        }
    }
    else
    {
        return false;
    }
}

FString FTextureSubTypeUtil::EncodeSubTypes(const TArray<ETextureSubType>& SubTypes)
{
    const UEnum* Enum = StaticEnum<ETextureSubType>();
    FString Result;
    for (auto SubType : SubTypes)
    {
        if (const FText Name = Enum->GetDisplayNameTextByValue(static_cast<int64>(SubType)); Name.IsEmpty())
        {
            Result.Reset();
            return Result;
        }
        else
        {
            if (!Result.IsEmpty())
            {
                Result.Append(TEXT(","));
            }
            Result.Append(Name.ToString());
        }
    }

    return Result;
}

FTextureSubTypeUtil::FTextureSubTypeUtil()
{
    // A private constructor so that it can not be accidentally instantiated.
}
