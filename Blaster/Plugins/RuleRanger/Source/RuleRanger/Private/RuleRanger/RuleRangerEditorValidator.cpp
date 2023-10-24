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
#include "RuleRangerDeveloperSettings.h"
#include "RuleRangerLogging.h"
#include "RuleRangerRule.h"

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
    const bool IsSave = EDataValidationUsecase::Save == DataValidationUsecase;
    const auto DeveloperSettings = GetMutableDefault<URuleRangerDeveloperSettings>();
    check(IsValid(DeveloperSettings));
    UE_LOG(RuleRanger,
           VeryVerbose,
           TEXT("CanValidateAsset(%s) discovered %d Rule Set(s)"),
           *InAsset->GetName(),
           DeveloperSettings->Rules.Num());
    for (auto RuleSetIt = DeveloperSettings->Rules.CreateIterator(); RuleSetIt; ++RuleSetIt)
    {
        if (const auto RuleSet = RuleSetIt->LoadSynchronous())
        {
            if (const auto Path = InAsset->GetPathName(); Path.StartsWith(RuleSet->Dir.Path))
            {
                UE_LOG(RuleRanger,
                       VeryVerbose,
                       TEXT("CanValidateAsset(%s) processing Rule Set %s"),
                       *InAsset->GetName(),
                       *RuleSet->GetName());
                for (const auto RulePtr : RuleSet->Rules)
                {
                    // ReSharper disable once CppTooWideScopeInitStatement
                    const auto Rule = RulePtr.Get();
                    if (!IsSave && Rule->bApplyOnValidate)
                    {
                        UE_LOG(RuleRanger,
                               VeryVerbose,
                               TEXT("CanValidateAsset(%s) detected applicable rule %s in usecase %s."),
                               *InAsset->GetName(),
                               *Rule->GetName(),
                               *DescribeDataValidationUsecase(DataValidationUsecase));
                        return true;
                    }
                    else if (IsSave && Rule->bApplyOnSave)
                    {
                        UE_LOG(RuleRanger,
                               VeryVerbose,
                               TEXT("CanValidateAsset(%s) detected applicable rule %s in usecase %s."),
                               *InAsset->GetName(),
                               *Rule->GetName(),
                               *DescribeDataValidationUsecase(DataValidationUsecase));
                        return true;
                    }
                }
            }
        }
        else
        {
            UE_LOG(RuleRanger,
                   Error,
                   TEXT("CanValidateAsset(%s) failed to load rule set %s."),
                   *InAsset->GetName(),
                   *RuleSetIt->GetAssetName());
        }
    }
    // If we get here there is no rules that apply to the asset
    return false;
}

EDataValidationResult URuleRangerEditorValidator::ValidateLoadedAsset_Implementation(UObject* InAsset,
                                                                                     TArray<FText>& ValidationErrors)
{
    if (IsValid(InAsset))
    {
        if (!ActionContext)
        {
            UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerEditorSubsystem: Creating the initial ActionContext"));
            ActionContext = NewObject<UActionContextImpl>(this, UActionContextImpl::StaticClass());
        }

        const bool IsSave = EDataValidationUsecase::Save == DataValidationUsecase;
        const auto DeveloperSettings = GetMutableDefault<URuleRangerDeveloperSettings>();
        check(IsValid(DeveloperSettings));
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("OnAssetValidate(%s) discovered %d Rule Set(s)"),
               *InAsset->GetName(),
               DeveloperSettings->Rules.Num());
        int RuleSetIndex = 0;
        for (auto RuleSetIt = DeveloperSettings->Rules.CreateIterator(); RuleSetIt; ++RuleSetIt)
        {
            RuleSetIndex++;
            const auto RuleSet = RuleSetIt->LoadSynchronous();
            if (!IsValid(RuleSet))
            {
                UE_LOG(RuleRanger,
                       Error,
                       TEXT("OnAssetValidate(%s) detected invalid RuleSet at index %d."),
                       *InAsset->GetName(),
                       RuleSetIndex);
            }
            if (const auto Path = InAsset->GetPathName(); Path.StartsWith(RuleSet->Dir.Path))
            {
                UE_LOG(RuleRanger,
                       VeryVerbose,
                       TEXT("OnAssetValidate(%s) processing Rule Set %s"),
                       *InAsset->GetName(),
                       *RuleSet->GetName());
                int RuleIndex = 0;
                for (const auto RulePtr : RuleSet->Rules)
                {
                    RuleIndex++;
                    // ReSharper disable once CppTooWideScopeInitStatement
                    const auto Rule = RulePtr.Get();
                    if (!IsValid(Rule))
                    {
                        UE_LOG(RuleRanger,
                               Error,
                               TEXT("OnAssetValidate(%s) detected invalid rule at index %d in RuleSet %s."),
                               *InAsset->GetName(),
                               RuleIndex,
                               *RuleSet->GetName());
                    }
                    else if ((!IsSave && Rule->bApplyOnValidate) || (IsSave && Rule->bApplyOnSave))
                    {
                        UE_LOG(RuleRanger,
                               VeryVerbose,
                               TEXT("OnAssetValidate(%s) detected applicable rule %s."),
                               *InAsset->GetName(),
                               *Rule->GetName());

                        ActionContext->ResetContext(InAsset,
                                                    IsSave ? ERuleRangerActionTrigger::AT_Save
                                                           : ERuleRangerActionTrigger::AT_Validate);

                        TScriptInterface<IRuleRangerActionContext> ScriptInterfaceActionContext(ActionContext);
                        Rule->Apply(ScriptInterfaceActionContext, InAsset);

                        ActionContext->EmitMessageLogs();

                        for (int i = 0; i < ActionContext->GetWarningMessages().Num(); i++)
                        {
                            AssetWarning(InAsset, ActionContext->GetWarningMessages()[i]);
                        }
                        for (int i = 0; i < ActionContext->GetErrorMessages().Num(); i++)
                        {
                            AssetFails(InAsset, ActionContext->GetErrorMessages()[i], ValidationErrors);
                        }
                        for (int i = 0; i < ActionContext->GetFatalMessages().Num(); i++)
                        {
                            AssetFails(InAsset, ActionContext->GetFatalMessages()[i], ValidationErrors);
                        }
                        ActionContext->ClearContext();
                    }
                }
            }
        }

        if (EDataValidationResult::NotValidated == GetValidationResult())
        {
            AssetPasses(InAsset);
        }
        return GetValidationResult();
    }
    else
    {
        return EDataValidationResult::NotValidated;
    }
}
