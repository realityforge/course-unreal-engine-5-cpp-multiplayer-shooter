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
#include "RuleRanger/RuleRangerEditorValidator.h"
#include "ActionContextImpl.h"
#include "RuleRangerActionContext.h"
#include "RuleRangerLogging.h"
#include "RuleRangerRule.h"
#include "RuleRanger/RuleRangerEditorSubsystem.h"

URuleRangerEditorValidator::URuleRangerEditorValidator()
{
    bIsEnabled = true;
}

// Cached value ... assuming this is valid and that CanValidateAsset is only invoked after
// CanValidate invoked and there is only one instance ... so maybe this horrible hack is ok?
static EDataValidationUsecase DataValidationUsecase{ EDataValidationUsecase::None };

static FString DescribeDataValidationUsecase(const EDataValidationUsecase InDataValidationUsecase)
{
    switch (InDataValidationUsecase)
    {
        case EDataValidationUsecase::None:
            return TEXT("None");
        case EDataValidationUsecase::Manual:
            return TEXT("Manual");
        case EDataValidationUsecase::Commandlet:
            return TEXT("Commandlet");
        case EDataValidationUsecase::Save:
            return TEXT("Save");
        case EDataValidationUsecase::PreSubmit:
            return TEXT("PreSubmit");
        case EDataValidationUsecase::Script:
        default:
            return TEXT("Script");
    }
}

bool URuleRangerEditorValidator::CanValidate_Implementation(const EDataValidationUsecase InUsecase) const
{
    UE_LOG(RuleRanger, Verbose, TEXT("CanValidate(%s)"), *DescribeDataValidationUsecase(InUsecase));
    DataValidationUsecase = InUsecase;
    return true;
}

bool URuleRangerEditorValidator::CanValidateAsset_Implementation(UObject* InAsset) const
{

    URuleRangerEditorSubsystem* SubSystem = GEditor
        ? GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>()
        : nullptr;
    return SubSystem
        ? SubSystem->IsMatchingRulePresent(InAsset,
                                           [this](URuleRangerRule* Rule, UObject* InObject) {
                                               return WillRuleRun(Rule, InObject);
                                           })
        : false;
}

EDataValidationResult URuleRangerEditorValidator::ValidateLoadedAsset_Implementation(
    UObject* InAsset,
    TArray<FText>& ValidationErrors)
{
    if (!ActionContext)
    {
        UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerEditorSubsystem: Creating the initial ActionContext"));
        ActionContext = NewObject<UActionContextImpl>(this, UActionContextImpl::StaticClass());
    }

    // ReSharper disable once CppTooWideScope
    const auto SubSystem = GEditor
        ? GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>()
        : nullptr;
    if (SubSystem)
    {
        TArray<FText>& ValidationErrors2 = ValidationErrors;
        SubSystem->ProcessRule(InAsset,
                               [this,ValidationErrors2](URuleRangerRule* Rule, UObject* InObject) mutable {
                                   return ProcessRule(ValidationErrors2, Rule, InObject);
                               });
    }
    else
    {
        UE_LOG(RuleRanger,
               Error,
               TEXT("OnAssetValidate(%s) unable to locate RuleRangerEditorSubsystem."),
               *InAsset->GetName());
    }

    if (EDataValidationResult::NotValidated == GetValidationResult())
    {
        AssetPasses(InAsset);
    }
    return GetValidationResult();
}

bool URuleRangerEditorValidator::ProcessRule(TArray<FText>& ValidationErrors, URuleRangerRule* Rule, UObject* InObject)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const bool bIsSave = EDataValidationUsecase::Save == DataValidationUsecase;
    if ((!bIsSave && Rule->bApplyOnValidate) || (bIsSave && Rule->bApplyOnSave))
    {
        UE_LOG(RuleRanger,
               Verbose,
               TEXT("OnAssetValidate(%s) detected applicable rule %s."),
               *InObject->GetName(),
               *Rule->GetName());

        ActionContext->ResetContext(InObject,
                                    bIsSave
                                    ? ERuleRangerActionTrigger::AT_Save
                                    : ERuleRangerActionTrigger::AT_Validate);

        TScriptInterface<IRuleRangerActionContext> ScriptInterfaceActionContext(ActionContext);
        Rule->Apply(ScriptInterfaceActionContext, InObject);

        ActionContext->EmitMessageLogs();

        for (int i = 0; i < ActionContext->GetWarningMessages().Num(); i++)
        {
            AssetWarning(InObject, ActionContext->GetWarningMessages()[i]);
        }
        for (int i = 0; i < ActionContext->GetErrorMessages().Num(); i++)
        {
            AssetFails(InObject, ActionContext->GetErrorMessages()[i], ValidationErrors);
        }
        for (int i = 0; i < ActionContext->GetFatalMessages().Num(); i++)
        {
            AssetFails(InObject, ActionContext->GetFatalMessages()[i], ValidationErrors);
        }
        return ActionContext->GetFatalMessages().Num() > 0 ? false : true;
    }
    else
    {
        return true;
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable 2 CppParameterMayBeConstPtrOrRef
bool URuleRangerEditorValidator::WillRuleRun(URuleRangerRule* Rule, UObject* InObject) const
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const bool bIsSave = EDataValidationUsecase::Save == DataValidationUsecase;
    if (!bIsSave && Rule->bApplyOnValidate)
    {
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("CanValidateAsset(%s) detected applicable rule %s in usecase %s."),
               *InObject->GetName(),
               *Rule->GetName(),
               *DescribeDataValidationUsecase(DataValidationUsecase));
        return true;
    }
    else if (bIsSave && Rule->bApplyOnSave)
    {
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("CanValidateAsset(%s) detected applicable rule %s in usecase %s."),
               *InObject->GetName(),
               *Rule->GetName(),
               *DescribeDataValidationUsecase(DataValidationUsecase));
        return true;
    }
    else
    {
        return false;
    }
}
