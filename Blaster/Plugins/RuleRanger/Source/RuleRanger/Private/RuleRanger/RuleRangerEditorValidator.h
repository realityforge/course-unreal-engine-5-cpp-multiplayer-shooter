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
#include "DataValidationModule.h"
#include "EditorValidatorBase.h"
#include "RuleRangerEditorValidator.generated.h"

class URuleRangerRuleSet;
class URuleRangerConfig;
class URuleRangerActionContext;
class URuleRangerRule;

/**
 * Validator extension that applies the RuleRanger rules in a validation mode.
 */
UCLASS()
class URuleRangerEditorValidator : public UEditorValidatorBase
{
    GENERATED_BODY()

public:
    URuleRangerEditorValidator();
    virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData,
                                                 UObject* InAsset,
                                                 FDataValidationContext& InContext) const override;
    virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData,
                                                                     UObject* InAsset,
                                                                     FDataValidationContext& Context) override;

private:
    UPROPERTY(VisibleAnywhere)
    URuleRangerActionContext* ActionContext{ nullptr };

    /**
     * Function invoked when each rule is applied to an object.
     *
     * @param Config The config that transitively contained the Rule.
     * @param RuleSet The RuleSet that directly contained the Rule.
     * @param Rule The rule to apply.
     * @param InObject the object to apply rule to.
     * @param Context the validation context.
     * @return true to keep processing, false if no more rules should be applied to object.
     */
    bool ProcessRule(URuleRangerConfig* const Config,
                     URuleRangerRuleSet* const RuleSet,
                     URuleRangerRule* Rule,
                     UObject* InObject,
                     FDataValidationContext& Context);

    bool WillRuleRunInDataValidationUsecase(const URuleRangerRule* Rule,
                                            const UObject* InObject,
                                            EDataValidationUsecase DataValidationUsecase) const;
};
