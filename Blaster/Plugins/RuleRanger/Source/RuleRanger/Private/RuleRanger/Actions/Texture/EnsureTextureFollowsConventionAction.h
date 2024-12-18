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
#include "RuleRanger/DataTypes/RuleRangerTextureConvention.h"
#include "RuleRangerAction.h"
#include "Texture2DActionBase.h"
#include "EnsureTextureFollowsConventionAction.generated.h"

enum class ETextureResolutionConstraint : uint8;

/**
 * Action to check that a Texture complies with the conventions specified in DataTable.
 */
UCLASS(DisplayName = "Ensure Texture follows Convention")
class RULERANGER_API UEnsureTextureFollowsConventionAction final : public UTexture2DActionBase
{
    GENERATED_BODY()

    /** The array of tables that contains the texture conventions */
    UPROPERTY(EditAnywhere,
              meta = (RequiredAssetDataTags = "RowStructure=/Script/RuleRanger.RuleRangerTextureConvention"))
    TArray<TObjectPtr<UDataTable>> ConventionsTables;

    /** Cache of DataTables from Config. */
    UPROPERTY(Transient)
    TArray<TObjectPtr<UDataTable>> ConfigConventionsTables;

    /** Cache for looking up rules. */
    TMap<FName, FRuleRangerTextureConvention> ConventionsCache;

    /** Handle for delegate called when any object modified in editor. */
    FDelegateHandle OnObjectModifiedDelegateHandle;

    /**
     * Flag controlling whether the action will attempt to fix the texture.
     * If there are multiple valid values, then the first in the list will be applied.
     */
    UPROPERTY(EditAnywhere)
    bool bApplyFix{ false };

    /** Callback when any object is modified in the editor. */
    void ResetCacheIfTableModified(UObject* Object);

    /** Method to clear cache. */
    void ResetConventionsCache();

    /** Method to build cache if necessary. */
    void RebuildConventionsCacheIfNecessary();

    /** Method to build ConfigConventionsTables. */
    void RebuildConfigConventionsTables(const URuleRangerActionContext* ActionContext);

    /**
     * Method to check that the texture has resolution that is power of two.
     *
     * @param ActionContext the Context.
     * @param Texture The Texture to Check.
     */
    void CheckPowerOfTwo(URuleRangerActionContext* ActionContext, const UTexture2D* Texture) const;

    /**
     * Method to check that the texture  complies with specified texture resolution constraint.
     *
     * @param ActionContext the Context.
     * @param Constraint the constraint.
     * @param Texture The Texture to Check.
     */
    void CheckDivisibleConstraint(URuleRangerActionContext* ActionContext,
                                  const ETextureResolutionConstraint Constraint,
                                  const UTexture2D* Texture) const;
    void PerformSetVariantMetaDataCheck(URuleRangerActionContext* ActionContext,
                                        UTexture2D* Texture,
                                        const FName& ConventionKey) const;
    void PerformNameSuffixCheck(URuleRangerActionContext* ActionContext,
                                UTexture2D* Texture,
                                const FRuleRangerTextureConvention* Convention);
    void PerformTextureResolutionConstraintCheck(URuleRangerActionContext* ActionContext,
                                                 const UTexture2D* Texture,
                                                 const FRuleRangerTextureConvention* Convention) const;
    void PerformTextureGroupCheck(URuleRangerActionContext* ActionContext,
                                  UTexture2D* Texture,
                                  FRuleRangerTextureConvention* Convention) const;
    void PerformColorSpaceCheck(URuleRangerActionContext* ActionContext,
                                UTexture2D* Texture,
                                const FRuleRangerTextureConvention* Convention) const;
    void PerformMipGenSettingsCheck(URuleRangerActionContext* ActionContext,
                                    UTexture2D* Texture,
                                    const FRuleRangerTextureConvention* Convention) const;
    void PerformTextureCompressionCheck(URuleRangerActionContext* ActionContext,
                                        UTexture2D* Texture,
                                        FRuleRangerTextureConvention* Convention) const;
    FName FindVariantBySuffix(const UTexture2D* Texture);

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
