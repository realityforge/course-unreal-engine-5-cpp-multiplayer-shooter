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

class UActionContextImpl;

/**
 * Validator extension that applies the RuleRanger rules in a validation mode.
 */
UCLASS()
class URuleRangerEditorValidator : public UEditorValidatorBase
{
    GENERATED_BODY()

public:
    URuleRangerEditorValidator();
    virtual bool CanValidate_Implementation(const EDataValidationUsecase InUsecase) const override;
    virtual bool CanValidateAsset_Implementation(UObject* InAsset) const override;
    virtual EDataValidationResult ValidateLoadedAsset_Implementation(UObject* InAsset,
                                                                     TArray<FText>& ValidationErrors) override;

private:
    UPROPERTY(VisibleAnywhere)
    UActionContextImpl* ActionContext{ nullptr };
};
