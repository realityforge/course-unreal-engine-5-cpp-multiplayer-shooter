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
#include "Engine/DataTable.h"
#include "RuleRangerAction.h"
#include "TextureSubType.h"
#include "EnsureTextureSubTypePresentAction.generated.h"

/**
 * The structure defining naming conventions for Texture extensions.
 */
USTRUCT(BlueprintType)
struct FTextureSubTypeNameConvention final : public FTableRowBase
{
    GENERATED_BODY()

    /**
     * The variant/subtype that this rule applies to.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETextureSubType SubType{ ETextureSubType::AT_BaseColor };

    /** The prefix to add to the name (if any). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Text{ TEXT("") };

    bool DoesTextMatchAtIndex(const FString& InText, const int32 Index) const;

    FORCEINLINE bool operator<(const FTextureSubTypeNameConvention& Other) const
    {
        const int LengthDifference = Text.Len() - Other.Text.Len();
        return 0 == LengthDifference ? Text < Other.Text : LengthDifference > 0;
    }
};

/**
 * Action to check that a Texture confirms to desired naming conventions.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"),
       Blueprintable,
       BlueprintType,
       CollapseCategories,
       DefaultToInstanced,
       EditInlineNew)
class RULERANGER_API UEnsureTextureSubTypePresentAction final : public URuleRangerAction
{
    GENERATED_BODY()

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() override;

    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

protected:
    void ApplyRuleToTexture(URuleRangerActionContext* ActionContext, const UTexture2D* Texture);
    void ApplyRuleToTextureWithSubTypes(URuleRangerActionContext* ActionContext,
                                        const UTexture2D* Texture,
                                        const TArray<ETextureSubType>& SubTypes) const;

private:
    /** The table that contains the object naming rules */
    UPROPERTY(EditAnywhere,
              BlueprintReadWrite,
              Category = "Rule Ranger",
              meta = (ExposeOnSpawn,
                      AllowPrivateAccess,
                      RequiredAssetDataTags = "RowStructure=/Script/RuleRanger.TextureSubTypeNameConvention"))
    UDataTable* NameConventionsTable{ nullptr };

    /** Should the action issue a message log when it attempts to process a Texture that has no naming convention? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    bool bNotifyIfNameConventionMissing;

    /** Cache for looking up rules. */
    TArray<FTextureSubTypeNameConvention> NameConventionsCache;

    /** Handle for delegate called when any object modified in editor. */
    FDelegateHandle OnObjectModifiedDelegateHandle;

    /** Callback when any object is modified in the editor. */
    void ResetCacheIfTableModified(UObject* Object);

    /** Method to clear cache. */
    void ResetNameConventionsCache();

    /** Method to build cache if necessary. */
    void RebuildNameConventionsCacheIfNecessary();

    TArray<ETextureSubType> ExtractSubTypes(const FString& Name);
};
