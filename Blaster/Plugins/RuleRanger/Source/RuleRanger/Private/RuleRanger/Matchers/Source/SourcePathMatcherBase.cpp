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
#include "SourcePathMatcherBase.h"
#include "EditorFramework/AssetImportData.h"
#include "UObject/PropertyAccessUtil.h"

bool USourcePathMatcherBase::Test_Implementation(UObject* Object)
{
    if (IsValid(Object))
    {
        const FName AssetImportDataPropName = FName("AssetImportData");
        if (const auto ObjectProp = PropertyAccessUtil::FindPropertyByName(AssetImportDataPropName, Object->GetClass()))
        {
            const FProperty* Prop{ nullptr };
            void* Value{ nullptr };
            const auto Result =
                PropertyAccessUtil::GetPropertyValue_Object(ObjectProp, Object, Prop, Value, INDEX_NONE);
            if (EPropertyAccessResultFlags::Success == Result)
            {
                // Should we be matching all filenames?
                const auto AssetImportData = static_cast<const UAssetImportData*>(Value);
                return Match(Object, AssetImportData->GetFirstFilename(), bCaseSensitive);
            }
        }
    }
    return false;
}

bool USourcePathMatcherBase::Match(UObject* Object, const FString& SourcePath, bool bInCaseSensitive)
{
    check(false);
    return false;
}
