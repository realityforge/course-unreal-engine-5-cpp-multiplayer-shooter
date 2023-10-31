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

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerEditorSubsystem::OnAssetPostImport([[maybe_unused]] UFactory* Factory, UObject* Object)
{
    const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();

    // Use a metadata tag when we have imported an asset so that when we try to reimport asset we can
    // identify this through the presence of tag.
    const bool bIsReimport = Subsystem && Subsystem->GetMetadataTag(Object, ImportMarkerKey) == ImportMarkerValue;

    ProcessRule(Object,
                [this,bIsReimport](URuleRangerRule* Rule, UObject* InObject) {
                    return ProcessOnAssetPostImportRule(bIsReimport, Rule, InObject);
                });
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerEditorSubsystem::ProcessRule(UObject* Object, const RuleRangerRuleFn& ProcessRuleFunction)
{
    if (IsValid(Object))
    {
        if (!ActionContext)
        {
            UE_LOG(RuleRanger, Verbose, TEXT("RuleRangerEditorSubsystem: Creating the initial ActionContext"));
            ActionContext = NewObject<UActionContextImpl>(this, UActionContextImpl::StaticClass());
        }

        const auto DeveloperSettings = GetMutableDefault<URuleRangerDeveloperSettings>();
        check(IsValid(DeveloperSettings));
        UE_LOG(RuleRanger,
               Verbose,
               TEXT("Located %d Rule Set Scope(s) when discovering rules for object %s"),
               DeveloperSettings->RuleSetScopes.Num(),
               *Object->GetName());
        for (auto RuleSetScopeIt = DeveloperSettings->RuleSetScopes.CreateIterator(); RuleSetScopeIt; ++RuleSetScopeIt)
        {
            if (const auto RuleSetScope = RuleSetScopeIt->LoadSynchronous())
            {
                if (const auto Path = Object->GetPathName(); RuleSetScope->ScopeMatches(Path))
                {
                    for (const auto RuleSetIt = RuleSetScope->RuleSets.CreateIterator(); RuleSetScopeIt; ++
                         RuleSetScopeIt)
                    {
                        if (const auto RuleSet = RuleSetIt->Get())
                        {
                            UE_LOG(RuleRanger,
                                   Verbose,
                                   TEXT("Processing Rule Set %s for object %s"),
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
                                               Verbose,
                                               TEXT("Rule %s indicated that following rules should be skipped for %s"),
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
                                           TEXT( "Invalid Rule skipped at index %d in " "rule set '%s' "
                                               "from scope '%s' when analyzing object '%s'" ),
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
                            UE_LOG(RuleRanger,
                                   Error,
                                   TEXT("Invalid RuleSet skipped when processing rules for %s in scope %s"),
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

bool URuleRangerEditorSubsystem::IsMatchingRulePresent(UObject* Object, const RuleRangerRuleFn& ProcessRuleFunction)
{
     if (IsValid(Object))
    {
        const auto DeveloperSettings = GetMutableDefault<URuleRangerDeveloperSettings>();
        check(IsValid(DeveloperSettings));
        UE_LOG(RuleRanger,
               Verbose,
               TEXT("Located %d Rule Set Scope(s) when discovering rules for object %s"),
               DeveloperSettings->RuleSetScopes.Num(),
               *Object->GetName());
        for (auto RuleSetScopeIt = DeveloperSettings->RuleSetScopes.CreateIterator(); RuleSetScopeIt; ++RuleSetScopeIt)
        {
            if (const auto RuleSetScope = RuleSetScopeIt->LoadSynchronous())
            {
                if (const auto Path = Object->GetPathName(); RuleSetScope->ScopeMatches(Path))
                {
                    for (const auto RuleSetIt = RuleSetScope->RuleSets.CreateIterator(); RuleSetScopeIt; ++
                         RuleSetScopeIt)
                    {
                        if (const auto RuleSet = RuleSetIt->Get())
                        {
                            UE_LOG(RuleRanger,
                                   Verbose,
                                   TEXT("Processing Rule Set %s for object %s"),
                                   *RuleSet->GetName(),
                                   *Object->GetName());
                            int RuleIndex = 0;
                            for (const auto RulePtr : RuleSet->Rules)
                            {
                                // ReSharper disable once CppTooWideScopeInitStatement
                                if (const auto Rule = RulePtr.Get(); IsValid(Rule))
                                {
                                    if(ProcessRuleFunction(Rule, Object))
                                    {
                                        return true;
                                    }
                                }
                                else
                                {
                                    UE_LOG(RuleRanger,
                                           Error,
                                           TEXT( "Invalid Rule skipped at index %d in " "rule set '%s' "
                                               "from scope '%s' when analyzing object '%s'" ),
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
                            UE_LOG(RuleRanger,
                                   Error,
                                   TEXT("Invalid RuleSet skipped when processing rules for %s in scope %s"),
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
    return false;
}

bool URuleRangerEditorSubsystem::ProcessOnAssetPostImportRule(const bool bIsReimport,
                                                              URuleRangerRule* Rule,
                                                              UObject* InObject)
{
    check(ActionContext);

    if ((!bIsReimport && Rule->bApplyOnImport) || (bIsReimport && Rule->bApplyOnReimport))
    {
        UE_LOG(RuleRanger,
               Verbose,
               TEXT("OnAssetPostImport(%s) applying rule %s during %s."),
               *InObject->GetName(),
               *Rule->GetName(),
               bIsReimport ? TEXT("reimport") : TEXT("import"));
        const ERuleRangerActionTrigger Trigger =
            bIsReimport
            ? ERuleRangerActionTrigger::AT_Reimport
            : ERuleRangerActionTrigger::AT_Import;
        ActionContext->ResetContext(InObject, Trigger);

        TScriptInterface<IRuleRangerActionContext> ScriptInterfaceActionContext(ActionContext);
        Rule->Apply(ScriptInterfaceActionContext, InObject);

        ActionContext->EmitMessageLogs();
        const auto State = ActionContext->GetState();
        if (ERuleRangerActionState::AS_Fatal == State)
        {
            UE_LOG(RuleRanger,
                   Verbose,
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
                   Verbose,
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
               Verbose,
               TEXT("OnAssetPostImport(%s) skipped rule %s as flag on "
                   "rule does not enable rule during %s."),
               *InObject->GetName(),
               *Rule->GetName(),
               bIsReimport ? TEXT("reimport") : TEXT("import"));
    }
    return true;
}
