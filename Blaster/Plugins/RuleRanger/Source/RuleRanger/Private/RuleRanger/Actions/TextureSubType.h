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

#pragma once

#include "CoreMinimal.h"
#include "TextureSubType.generated.h"

/**
 * The various texture subtypes in use.
 */
UENUM(BlueprintType)
enum class ETextureSubType : uint8
{
    AT_Roughness UMETA(DisplayName = "Roughness"),
    AT_Opacity UMETA(DisplayName = "Alpha/Opacity"),
    AT_AmbientOcclusion UMETA(DisplayName = "Ambient Occlusion"),
    AT_BaseColor UMETA(DisplayName = "Base Color"),
    AT_Specular UMETA(DisplayName = "Specular"),
    AT_Metallic UMETA(DisplayName = "Metallic"),
    AT_Normal UMETA(DisplayName = "Normal"),
    AT_Emissive UMETA(DisplayName = "Emissive"),
    AT_Height UMETA(DisplayName = "Height"),
    AT_Mask UMETA(DisplayName = "Mask"),
    AT_FlowMap UMETA(DisplayName = "Flow Map"),
    AT_Displacement UMETA(DisplayName = "Displacement"),
    AT_LightMap UMETA(DisplayName = "Light Map"),

    AT_Max UMETA(Hidden)
};

/**
 * A simple utility class for interacting with TextureSubTypes.
 */
class FTextureSubTypeUtil
{
public:
    /**
     * Extract the TextureSubTypes MetaData property from the specified object.
     *
     * @param Object the object.
     * @return The array of TextureSubtypes.
     */
    static TArray<ETextureSubType> ExtractFromMetaData(const UObject* Object);

    /**
     * Check whether the SubTypes are encoded in the MetaData.
     *
     * @param Object the object.
     * @param SubTypes the SubTypes.
     * @return True if match, false if missing or do not match.
     */
    static bool DoesMetaDataMatch(const UObject* Object, const TArray<ETextureSubType>& SubTypes);

    /**
     * Encode the SubTypes into a text and set it as the Metadata property.
     * If the SubTypes is Empty then the MetaData property will be removed if present.
     *
     * @param Object the object.
     * @param SubTypes the SubTypes.
     * @return True on success, false if unable to set MetaData
     */
    static bool EncodeInMetaData(const UObject* Object, const TArray<ETextureSubType>& SubTypes);

    /**
     * Encode the SubTypes as a string.
     *
     * @param SubTypes the SubTypes.
     * @return The String.
     */
    static FString EncodeSubTypes(const TArray<ETextureSubType>& SubTypes);

private:
    /**
     * The metadata key used to store SubTypes.
     */
    static const FName MetaDataKey;

    FTextureSubTypeUtil();

public:
    FORCEINLINE static const FName& GetMetaDataKey() { return MetaDataKey; }
};