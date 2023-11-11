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
#include "RuleRanger/RuleRangerEditorSubsystem.h"
#include "ActionContextImpl.h"
#include "Editor.h"
#include "RuleRangerDeveloperSettings.h"
#include "RuleRangerLogging.h"
#include "RuleRangerRule.h"
#include "RuleRangerRuleSet.h"
#include "RuleRangerRuleSetScope.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Subsystems/ImportSubsystem.h"

static FName ImportMarkerKey = FName(TEXT("RuleRanger.ImportProcessed"));
static FString ImportMarkerValue = FString(TEXT("True"));

void URuleRangerEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    // Register delegate for OnAssetPostImport callback
    if (const auto Subsystem = GEditor->GetEditorSubsystem<UImportSubsystem>())
    {
        OnAssetPostImportDelegateHandle =
            Subsystem->OnAssetPostImport.AddUObject(this, &URuleRangerEditorSubsystem::OnAssetPostImport);
    }
}

void URuleRangerEditorSubsystem::Deinitialize()
{
    // Deregister delegate for OnAssetPostImport callback
    if (OnAssetPostImportDelegateHandle.IsValid())
    {
        if (const auto Subsystem = GEditor->GetEditorSubsystem<UImportSubsystem>())
        {
            Subsystem->OnAssetPostImport.Remove(OnAssetPostImportDelegateHandle);
            OnAssetPostImportDelegateHandle.Reset();
        }
    }
}

void URuleRangerEditorSubsystem::ScanObject(UObject* InObject)
{
    ProcessRule(InObject,
                [this](URuleRangerRule* Rule, UObject* InObject) mutable { return ProcessDemandScan(Rule, InObject); });
}

void URuleRangerEditorSubsystem::ScanAndFixObject(UObject* InObject)
{
    ProcessRule(InObject, [this](URuleRangerRule* Rule, UObject* InObject) mutable {
        return ProcessDemandScanAndFix(Rule, InObject);
    });
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerEditorSubsystem::OnAssetPostImport([[maybe_unused]] UFactory* Factory, UObject* Object)
{
    const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();

    // Use a metadata tag when we have imported an asset so that when we try to reimport asset we can
    // identify this through the presence of tag.
    const bool bIsReimport = Subsystem && Subsystem->GetMetadataTag(Object, ImportMarkerKey) == ImportMarkerValue;

    ProcessRule(Object, [this, bIsReimport](URuleRangerRule* Rule, UObject* InObject) {
        return ProcessOnAssetPostImportRule(bIsReimport, Rule, InObject);
    });
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerEditorSubsystem::ProcessRule(UObject* Object, const FRuleRangerRuleFn& ProcessRuleFunction)
{
    if (IsValid(Object))
    {
        if (!ActionContext)
        {
            UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerEditorSubsystem: Creating the initial ActionContext"));
            ActionContext = NewObject<UActionContextImpl>(this, UActionContextImpl::StaticClass());

            // TODO: This will not have correct type when first created
        }

        auto RuleRangerRuleSetScopes = GetActiveRuleSetScopes();
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("ProcessRule: Located %d Rule Set Scope(s) when discovering rules for object %s"),
               RuleRangerRuleSetScopes.Num(),
               *Object->GetName());
        for (auto RuleSetScopeIt = RuleRangerRuleSetScopes.CreateIterator(); RuleSetScopeIt; ++RuleSetScopeIt)
        {
            if (const auto RuleSetScope = RuleSetScopeIt->LoadSynchronous())
            {
                if (const auto Path = Object->GetPathName(); RuleSetScope->ScopeMatches(Path))
                {
                    for (auto RuleSetIt = RuleSetScope->RuleSets.CreateIterator(); RuleSetIt; ++RuleSetIt)
                    {
                        if (const auto RuleSet = RuleSetIt->Get())
                        {
                            UE_LOG(RuleRanger,
                                   VeryVerbose,
                                   TEXT("ProcessRule: Processing Rule Set %s for object %s"),
                                   *RuleSet->GetName(),
                                   *Object->GetName());
                            int RuleIndex = 0;
                            for (const auto RulePtr : RuleSet->Rules)
                            {
                                // ReSharper disable once CppTooWideScopeInitStatement
                                if (const auto Rule = RulePtr.Get(); IsValid(Rule))
                                {
                                    if (!ProcessRuleFunction(Rule, Object))
                                    {
                                        UE_LOG(RuleRanger,
                                               VeryVerbose,
                                               TEXT("ProcessRule: Rule %s indicated that following "
                                                    "rules should be skipped for %s"),
                                               *RuleSet->GetName(),
                                               *Object->GetName());
                                        ActionContext->ClearContext();
                                        return;
                                    }
                                }
                                else
                                {
                                    UE_LOG(RuleRanger,
                                           Error,
                                           TEXT("ProcessRule: Invalid Rule skipped at index %d in "
                                                "rule set '%s' "
                                                "from scope '%s' when analyzing object '%s'"),
                                           RuleIndex,
                                           *RuleSet->GetName(),
                                           *RuleSetScope->GetName(),
                                           *Object->GetName());
                                }
                                RuleIndex++;
                            }
                        }
                        else
                        {
                            UE_LOG(
                                RuleRanger,
                                Error,
                                TEXT("ProcessRule: Invalid RuleSet skipped when processing rules for %s in scope %s"),
                                *Object->GetName(),
                                *RuleSetScope->GetName());
                        }
                    }
                }
            }
            else
            {
                UE_LOG(RuleRanger,
                       Error,
                       TEXT("Invalid RuleSetScope skipped when processing rules for %s"),
                       *Object->GetName());
            }
        }
    }
    ActionContext->ClearContext();
}

bool URuleRangerEditorSubsystem::IsMatchingRulePresent(UObject* InObject, const FRuleRangerRuleFn& ProcessRuleFunction)
{
    if (IsValid(InObject))
    {
        auto RuleRangerRuleSetScopes = GetActiveRuleSetScopes();
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("IsMatchingRulePresent: Located %d Rule Set Scope(s) when discovering rules for object %s"),
               RuleRangerRuleSetScopes.Num(),
               *InObject->GetName());
        for (auto RuleSetScopeIt = RuleRangerRuleSetScopes.CreateIterator(); RuleSetScopeIt; ++RuleSetScopeIt)
        {
            if (const auto RuleSetScope = RuleSetScopeIt->LoadSynchronous())
            {
                if (const auto Path = InObject->GetPathName(); RuleSetScope->ScopeMatches(Path))
                {
                    for (auto RuleSetIt = RuleSetScope->RuleSets.CreateIterator(); RuleSetIt; ++RuleSetIt)
                    {
                        if (const auto RuleSet = RuleSetIt->Get())
                        {
                            UE_LOG(RuleRanger,
                                   VeryVerbose,
                                   TEXT("IsMatchingRulePresent: Processing Rule Set %s for object %s"),
                                   *RuleSet->GetName(),
                                   *InObject->GetName());
                            int RuleIndex = 0;
                            for (const auto RulePtr : RuleSet->Rules)
                            {
                                // ReSharper disable once CppTooWideScopeInitStatement
                                if (const auto Rule = RulePtr.Get(); IsValid(Rule))
                                {
                                    if (ProcessRuleFunction(Rule, InObject))
                                    {
                                        return true;
                                    }
                                }
                                else
                                {
                                    UE_LOG(RuleRanger,
                                           Error,
                                           TEXT("IsMatchingRulePresent: Invalid Rule skipped at index %d in "
                                                "rule set '%s' "
                                                "from scope '%s' when analyzing object '%s'"),
                                           RuleIndex,
                                           *RuleSet->GetName(),
                                           *RuleSetScope->GetName(),
                                           *InObject->GetName());
                                }
                                RuleIndex++;
                            }
                        }
                        else
                        {
                            UE_LOG(RuleRanger,
                                   Error,
                                   TEXT("IsMatchingRulePresent: Invalid RuleSet skipped when processing "
                                        "rules for %s in scope %s"),
                                   *InObject->GetName(),
                                   *RuleSetScope->GetName());
                        }
                    }
                }
            }
            else
            {
                UE_LOG(RuleRanger,
                       Error,
                       TEXT("IsMatchingRulePresent: Invalid RuleSetScope skipped when processing rules for %s"),
                       *InObject->GetName());
            }
        }
    }
    return false;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
TArray<TSoftObjectPtr<URuleRangerRuleSetScope>> URuleRangerEditorSubsystem::GetActiveRuleSetScopes()
{
    const auto DeveloperSettings = GetMutableDefault<URuleRangerDeveloperSettings>();
    check(IsValid(DeveloperSettings));
    return DeveloperSettings->RuleSetScopes;
}

bool URuleRangerEditorSubsystem::ProcessOnAssetPostImportRule(const bool bIsReimport,
                                                              URuleRangerRule* Rule,
                                                              UObject* InObject)
{
    check(ActionContext);

    if ((!bIsReimport && Rule->bApplyOnImport) || (bIsReimport && Rule->bApplyOnReimport))
    {
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("OnAssetPostImport(%s) applying rule %s during %s."),
               *InObject->GetName(),
               *Rule->GetName(),
               bIsReimport ? TEXT("reimport") : TEXT("import"));
        const ERuleRangerActionTrigger Trigger =
            bIsReimport ? ERuleRangerActionTrigger::AT_Reimport : ERuleRangerActionTrigger::AT_Import;
        ActionContext->ResetContext(InObject, Trigger);

        TScriptInterface<IRuleRangerActionContext> ScriptInterfaceActionContext(ActionContext);
        Rule->Apply(ScriptInterfaceActionContext, InObject);

        ActionContext->EmitMessageLogs();
        const auto State = ActionContext->GetState();
        if (ERuleRangerActionState::AS_Fatal == State)
        {
            UE_LOG(RuleRanger,
                   VeryVerbose,
                   TEXT("OnAssetPostImport(%s) applied rule %s which resulted in fatal error. "
                        "Processing rules will not continue."),
                   *InObject->GetName(),
                   *Rule->GetName());
            ActionContext->ClearContext();
            return false;
        }
        else if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
        {
            UE_LOG(RuleRanger,
                   VeryVerbose,
                   TEXT("OnAssetPostImport(%s) applied rule %s which resulted in error. "
                        "Processing rules will not continue as ContinueOnError=False."),
                   *InObject->GetName(),
                   *Rule->GetName());
            ActionContext->ClearContext();
            return false;
        }
    }
    else
    {
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("OnAssetPostImport(%s) skipped rule %s as flag on "
                    "rule does not enable rule during %s."),
               *InObject->GetName(),
               *Rule->GetName(),
               bIsReimport ? TEXT("reimport") : TEXT("import"));
    }
    return true;
}

bool URuleRangerEditorSubsystem::ProcessDemandScan(URuleRangerRule* Rule, UObject* InObject)
{
    check(ActionContext);

    if (Rule->bApplyOnDemand)
    {
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("ProcessDemandScan(%s) applying rule %s."),
               *InObject->GetName(),
               *Rule->GetName());
        ActionContext->ResetContext(InObject, ERuleRangerActionTrigger::AT_Validate);

        TScriptInterface<IRuleRangerActionContext> ScriptInterfaceActionContext(ActionContext);
        Rule->Apply(ScriptInterfaceActionContext, InObject);

        ActionContext->EmitMessageLogs();
        const auto State = ActionContext->GetState();
        if (ERuleRangerActionState::AS_Fatal == State)
        {
            UE_LOG(RuleRanger,
                   VeryVerbose,
                   TEXT("ProcessDemandScan(%s) applied rule %s which resulted in fatal error. "
                        "Processing rules will not continue."),
                   *InObject->GetName(),
                   *Rule->GetName());
            ActionContext->ClearContext();
            return false;
        }
        else if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
        {
            UE_LOG(RuleRanger,
                   VeryVerbose,
                   TEXT("ProcessDemandScan(%s) applied rule %s which resulted in error. "
                        "Processing rules will not continue as ContinueOnError=False."),
                   *InObject->GetName(),
                   *Rule->GetName());
            ActionContext->ClearContext();
            return false;
        }
    }
    else
    {
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("ProcessDemandScan(%s) skipped rule %s as flag on "
                    "rule does not enable rule on demand."),
               *InObject->GetName(),
               *Rule->GetName());
    }
    return true;
}

bool URuleRangerEditorSubsystem::ProcessDemandScanAndFix(URuleRangerRule* Rule, UObject* InObject)
{
    check(ActionContext);

    if (Rule->bApplyOnDemand)
    {
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("ProcessDemandScanAndFix(%s) applying rule %s."),
               *InObject->GetName(),
               *Rule->GetName());
        ActionContext->ResetContext(InObject, ERuleRangerActionTrigger::AT_Fix);

        TScriptInterface<IRuleRangerActionContext> ScriptInterfaceActionContext(ActionContext);
        Rule->Apply(ScriptInterfaceActionContext, InObject);

        ActionContext->EmitMessageLogs();
        const auto State = ActionContext->GetState();
        if (ERuleRangerActionState::AS_Fatal == State)
        {
            UE_LOG(RuleRanger,
                   VeryVerbose,
                   TEXT("ProcessDemandScanAndFix(%s) applied rule %s which resulted in fatal error. "
                        "Processing rules will not continue."),
                   *InObject->GetName(),
                   *Rule->GetName());
            ActionContext->ClearContext();
            return false;
        }
        else if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
        {
            UE_LOG(RuleRanger,
                   VeryVerbose,
                   TEXT("ProcessDemandScanAndFix(%s) applied rule %s which resulted in error. "
                        "Processing rules will not continue as ContinueOnError=False."),
                   *InObject->GetName(),
                   *Rule->GetName());
            ActionContext->ClearContext();
            return false;
        }
    }
    else
    {
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("ProcessDemandScanAndFix(%s) skipped rule %s as flag on "
                    "rule does not enable rule on demand."),
               *InObject->GetName(),
               *Rule->GetName());
    }
    return true;
}
