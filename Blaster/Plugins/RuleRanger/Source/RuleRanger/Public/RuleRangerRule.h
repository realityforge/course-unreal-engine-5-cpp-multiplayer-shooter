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
#include "Engine/DataAsset.h"
#include "RuleRangerRule.generated.h"

class URuleRangerActionContext;
class URuleRangerAction;
class URuleRangerMatcher;

/**
 * The object that binds one or more matchers with one or more actions.
 */
UCLASS(AutoExpandCategories = ("Default", "Rule Ranger"), Blueprintable, BlueprintType, EditInlineNew)
class RULERANGER_API URuleRangerRule final : public UDataAsset
{
    GENERATED_BODY()

public:
    /**
     * An explanation of the rule.
     */
    UPROPERTY(EditAnywhere, Category = "Default", meta = (MultiLine))
    FString Description{ TEXT("") };

    /**
     * True to apply this rule when importing an asset initially.
     */
    UPROPERTY(EditAnywhere, Category = "Default")
    bool bApplyOnImport{ true };

    /**
     * True to apply this rule when reimporting an asset.
     */
    UPROPERTY(EditAnywhere, Category = "Default")
    bool bApplyOnReimport{ true };

    /**
     * True to apply this rule when the asset is validated.
     */
    UPROPERTY(EditAnywhere, Category = "Default")
    bool bApplyOnValidate{ true };

    /**
     * True to apply this rule when the asset is validated.
     */
    UPROPERTY(EditAnywhere, Category = "Default")
    bool bApplyOnSave{ true };

    /**
     * True to apply this rule when explicitly requested from the editor UI.
     */
    UPROPERTY(EditAnywhere, Category = "Default")
    bool bApplyOnDemand{ true };

    /**
     * A flag that controls whether the presence of an error will result in subsequent actions being skipped if an error
     * is detected.
     */
    UPROPERTY(EditAnywhere, Category = "Default")
    bool bContinueOnError{ false };

    /**
     * Priority used to order rules when multiple apply to the same ObjectType.
     */
    UPROPERTY(EditAnywhere, Category = "Default")
    int32 Priority{ 1000 };

    /** The matchers that an object MUST match before this rule is applied. */
    UPROPERTY(Instanced, EditAnywhere, Category = "Rule Ranger")
    TArray<TObjectPtr<URuleRangerMatcher>> Matchers;

    /** The actions that will be applied if the object is matched by the rule. */
    UPROPERTY(Instanced, EditAnywhere, Category = "Rule Ranger")
    TArray<TObjectPtr<URuleRangerAction>> Actions;

    /**
     * Apply the actions associated with the rule to the specified object.
     *
     * @param ActionContext the context in which the actions are invoked.
     * @param Object the object to apply the actions to.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Rule Ranger")
    void Apply(URuleRangerActionContext* ActionContext, UObject* Object);

    /**
     * Run the matchers associated with the rule to see if actions will be invoked by this rule or whether no matcher
     * match.
     *
     * @param ActionContext the context in which the actions are invoked.
     * @param Object the object to apply the actions to.
     * @return true if the rule matches, false otherwise.
     */
    bool Match(URuleRangerActionContext* ActionContext, UObject* Object) const;
};
