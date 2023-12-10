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
#include "NameConventionRenameAction.generated.h"

inline static FString NameConvention_DefaultVariant{ TEXT("") };

/**
 * The structure defining naming conventions for different asset types.
 */
USTRUCT(BlueprintType)
struct FNameConvention final : public FTableRowBase
{
    GENERATED_BODY();

    /** The object type that this name convention applied to. */
    UPROPERTY(EditAnywhere, meta = (AllowAbstract))
    TSoftClassPtr<UObject> ObjectType{ nullptr };

    /**
     * The variant/subtype to add to the name (if any).
     * This is expected to be derived before applying this rule.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Variant{ NameConvention_DefaultVariant };

    /** The prefix to add to the name (if any). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Prefix{ TEXT("") };

    /** The suffix to add to the name (if any). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Suffix{ TEXT("") };

    FORCEINLINE bool operator<(const FNameConvention& Other) const
    {
        return Variant.Equals(NameConvention_DefaultVariant)      ? false
            : Other.Variant.Equals(NameConvention_DefaultVariant) ? true
            : Prefix != Other.Prefix                              ? Prefix < Other.Prefix
                                                                  : Suffix < Other.Suffix;
    }
};

/**
 * Action to rename assets according to naming convention specified in a DataTable.
 * The action may optionally issue warnings if applied to an asset that has no NamingConvention specified.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"),
       Blueprintable,
       BlueprintType,
       CollapseCategories,
       DefaultToInstanced,
       EditInlineNew)
class RULERANGER_API UNameConventionRenameAction : public URuleRangerAction
{
    GENERATED_BODY()

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:
    /** The table that contains the object naming rules */
    UPROPERTY(EditAnywhere,
              BlueprintReadWrite,
              Category = "Rule Ranger",
              meta = (ExposeOnSpawn,
                      AllowPrivateAccess,
                      RequiredAssetDataTags = "RowStructure=/Script/RuleRanger.NameConvention"))
    UDataTable* NameConventionsTable{ nullptr };

    /** Should the action issue a message log when it attempts to process an object that has no naming convention? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    bool bNotifyIfNameConventionMissing;

    /** Cache for looking up rules. */
    TMap<TObjectPtr<UClass>, TArray<FNameConvention>> NameConventionsCache;

    /** Handle for delegate called when any object modified in editor. */
    FDelegateHandle OnObjectModifiedDelegateHandle;

    /** Callback when any object is modified in the editor. */
    void ResetCacheIfTableModified(UObject* Object);

    /** Method to clear cache. */
    void ResetNameConventionsCache();

    /** Method to build cache if necessary. */
    void RebuildNameConventionsCacheIfNecessary();
};
