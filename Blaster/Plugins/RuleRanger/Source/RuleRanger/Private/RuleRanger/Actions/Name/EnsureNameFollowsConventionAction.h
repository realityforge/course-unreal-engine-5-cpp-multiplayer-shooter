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
#include "EnsureNameFollowsConventionAction.generated.h"

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
    FString Prefix{ "" };

    /** The suffix to add to the name (if any). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Suffix{ "" };

    /** An explanation of the convention. */
    UPROPERTY(EditAnywhere)
    FString Description{ "" };

    FORCEINLINE bool operator<(const FNameConvention& Other) const
    {
        return Variant.Equals(NameConvention_DefaultVariant)      ? false
            : Other.Variant.Equals(NameConvention_DefaultVariant) ? true
            : Prefix != Other.Prefix                              ? Prefix < Other.Prefix
                                                                  : Suffix < Other.Suffix;
    }

    FORCEINLINE bool operator!=(const FNameConvention& Other) const { return !(*this == Other); }

    FORCEINLINE bool operator==(const FNameConvention& Other) const
    {
        return ObjectType == Other.ObjectType && Variant.Equals(Other.Variant) && Prefix.Equals(Other.Prefix)
            && Suffix.Equals(Other.Suffix);
    }
};

/**
 * Deprecated Name conventions that should be removed before applying other naming rules.
 *
 * These rules are either ones builtin to the engine (i.e. `_Inst` suffix when creating
 * instances of various types from within the editor) or were used historically. (Thus many
 * assets follow convention, and we want an easy way to rename them to the new conventions.)
 */
USTRUCT(BlueprintType)
struct FDeprecatedNameConvention final : public FTableRowBase
{
    GENERATED_BODY();

    /** The object type that this name convention applied to. */
    UPROPERTY(EditAnywhere, meta = (AllowAbstract))
    TSoftClassPtr<UObject> ObjectType{ nullptr };

    /** The prefix to remove from the name (if any). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Prefix{ "" };

    /** The suffix to remove from the name (if any). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Suffix{ "" };

    /** An explanation of the deprecation. */
    UPROPERTY(EditAnywhere)
    FString Description{ "" };

    FORCEINLINE bool operator<(const FDeprecatedNameConvention& Other) const
    {
        return Prefix != Other.Prefix ? Prefix < Other.Prefix : Suffix < Other.Suffix;
    }

    FORCEINLINE bool operator!=(const FDeprecatedNameConvention& Other) const { return !(*this == Other); }

    FORCEINLINE bool operator==(const FDeprecatedNameConvention& Other) const
    {
        return ObjectType == Other.ObjectType && Prefix.Equals(Other.Prefix) && Suffix.Equals(Other.Suffix);
    }
};

/**
 * Action to rename assets according to naming convention specified in a DataTable.
 * The action may optionally issue warnings if applied to an asset that has no NamingConvention specified.
 */
UCLASS(DisplayName = "Ensure Name Follows Convention")
class RULERANGER_API UEnsureNameFollowsConventionAction : public URuleRangerAction
{
    GENERATED_BODY()

    /** The array of tables that contains the deprecated naming rules */
    UPROPERTY(EditAnywhere, meta = (RequiredAssetDataTags = "RowStructure=/Script/RuleRanger.DeprecatedNameConvention"))
    TArray<TObjectPtr<UDataTable>> DeprecatedConventionsTables;

    /** The array of tables that contains the object naming rules */
    UPROPERTY(EditAnywhere, meta = (RequiredAssetDataTags = "RowStructure=/Script/RuleRanger.NameConvention"))
    TArray<TObjectPtr<UDataTable>> NameConventionsTables;

    /** Should the action issue a message log when it attempts to process an object that has no naming convention? */
    UPROPERTY(EditAnywhere)
    bool bNotifyIfNameConventionMissing{ false };

    /** Cache for looking up deprecated rules. */
    TMap<TObjectPtr<UClass>, TArray<FDeprecatedNameConvention>> DeprecatedConventionsCache;

    /** Cache for looking up rules. */
    TMap<TObjectPtr<UClass>, TArray<FNameConvention>> ConventionsCache;

    /** Handle for delegate called when any object modified in editor. */
    FDelegateHandle OnObjectModifiedDelegateHandle;

    /** Callback when any object is modified in the editor. */
    void ResetCachesIfTablesModified(UObject* Object);

    /** Method to clear cache. */
    void ResetCaches();

    /** Method to build convention cache if necessary. */
    void RebuildConventionCacheIfNecessary();

    /** Method to build deprecated convention cache if necessary. */
    void RebuildDeprecatedConventionCacheIfNecessary();

    bool FindMatchingNameConvention(URuleRangerActionContext* ActionContext,
                                    const UObject* Object,
                                    const TArray<UClass*>& Classes,
                                    const FString& Variant,
                                    FNameConvention& MatchingConvention) const;
    void RebuildCachesIfNecessary();

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
