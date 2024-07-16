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
#include "Editor.h"
#include "RuleRangerActionContext.h"
#include "RuleRangerConfig.h"
#include "RuleRangerDeveloperSettings.h"
#include "RuleRangerLogging.h"
#include "RuleRangerRule.h"
#include "RuleRangerRuleExclusion.h"
#include "RuleRangerRuleSet.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Subsystems/ImportSubsystem.h"

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

    const static FName NAME_ImportMarkerKey = FName(TEXT("RuleRanger.ImportProcessed"));
    // Use a metadata tag when we have imported an asset so that when we try to reimport asset we can
    // identify this through the presence of tag.
    const bool bIsReimport = Subsystem && Subsystem->GetMetadataTag(Object, NAME_ImportMarkerKey) == ImportMarkerValue;

    ProcessRule(Object, [this, bIsReimport](URuleRangerRule* Rule, UObject* InObject) {
        return ProcessOnAssetPostImportRule(bIsReimport, Rule, InObject);
    });
}

bool URuleRangerEditorSubsystem::ProcessRuleSetForObject(URuleRangerConfig* const Config,
                                                         URuleRangerRuleSet* const RuleSet,
                                                         TArray<URuleRangerRuleExclusion*> Exclusions,
                                                         UObject* Object,
                                                         const FRuleRangerRuleFn& ProcessRuleFunction)
{
    UE_LOG(RuleRanger,
           VeryVerbose,
           TEXT("ProcessRule: Processing Rule Set %s for object %s"),
           *RuleSet->GetName(),
           *Object->GetName());

    for (auto ExclusionIt = Exclusions.CreateIterator(); ExclusionIt; ++ExclusionIt)
    {
        if (const auto Exclusion = *ExclusionIt)
        {
            if (Exclusion->RuleSets.Contains(RuleSet))
            {
                UE_LOG(RuleRanger,
                       VeryVerbose,
                       TEXT("ProcessRule: Rule Set %s excluded for object %s due to exclusion rule. Reason: %s"),
                       *RuleSet->GetName(),
                       *Object->GetName(),
                       *Exclusion->Description.ToString());
                return true;
            }
        }
    }

    for (auto RuleSetIt = RuleSet->RuleSets.CreateIterator(); RuleSetIt; ++RuleSetIt)
    {
        if (const auto NestedRuleSet = RuleSetIt->Get())
        {
            UE_LOG(RuleRanger,
                   VeryVerbose,
                   TEXT("ProcessRule: Processing Nested Rule Set %s for object %s"),
                   *NestedRuleSet->GetName(),
                   *Object->GetName());
            if (!ProcessRuleSetForObject(Config, NestedRuleSet, Exclusions, Object, ProcessRuleFunction))
            {
                return false;
            }
            UE_LOG(RuleRanger,
                   VeryVerbose,
                   TEXT("ProcessRule: Completed processing of Nested Rule Set %s for object %s"),
                   *NestedRuleSet->GetName(),
                   *Object->GetName());
        }
        else
        {
            UE_LOG(RuleRanger,
                   Error,
                   TEXT("ProcessRule: Invalid RuleSet skipped when processing rules for %s in config %s"),
                   *Object->GetName(),
                   *Config->GetName());
        }
    }

    int RuleIndex = 0;
    for (const auto RulePtr : RuleSet->Rules)
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        if (const auto Rule = RulePtr.Get(); IsValid(Rule))
        {
            for (auto ExclusionIt = Exclusions.CreateIterator(); ExclusionIt; ++ExclusionIt)
            {
                if (const auto Exclusion = *ExclusionIt)
                {
                    if (Exclusion->Rules.Contains(Rule))
                    {
                        UE_LOG(RuleRanger,
                               VeryVerbose,
                               TEXT("ProcessRule: Rule %s excluded for object %s due to exclusion rule. Reason: %s"),
                               *Rule->GetName(),
                               *Object->GetName(),
                               *Exclusion->Description.ToString());
                        return true;
                    }
                }
            }

            if (!ProcessRuleFunction(Rule, Object))
            {
                UE_LOG(RuleRanger,
                       VeryVerbose,
                       TEXT("ProcessRule: Rule %s indicated that following rules should be skipped for %s"),
                       *RuleSet->GetName(),
                       *Object->GetName());
                ActionContext->ClearContext();
                return false;
            }
        }
        else
        {
            UE_LOG(RuleRanger,
                   Error,
                   TEXT("ProcessRule: Invalid Rule skipped at index %d in rule set '%s' "
                        "from config '%s' when analyzing object '%s'"),
                   RuleIndex,
                   *RuleSet->GetName(),
                   *Config->GetName(),
                   *Object->GetName());
        }
        RuleIndex++;
    }
    return true;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerEditorSubsystem::ProcessRule(UObject* Object, const FRuleRangerRuleFn& ProcessRuleFunction)
{
    if (IsValid(Object))
    {
        if (!ActionContext)
        {
            UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerEditorSubsystem: Creating the initial ActionContext"));
            ActionContext = NewObject<URuleRangerActionContext>(this, URuleRangerActionContext::StaticClass());

            // TODO: This will not have correct type when first created
        }

        auto Configs = GetCurrentRuleSetConfigs();
        const auto Path = Object->GetPathName();
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("ProcessRule: Located %d Rule Set Config(s) when discovering rules for object %s at %s"),
               Configs.Num(),
               *Object->GetName(),
               *Path);
        for (auto ConfigIt = Configs.CreateIterator(); ConfigIt; ++ConfigIt)
        {
            if (const auto Config = ConfigIt->LoadSynchronous())
            {
                if (Config->ConfigMatches(Path))
                {

                    TArray<URuleRangerRuleExclusion*> Exclusions;
                    for (auto ExclusionIt = Config->Exclusions.CreateIterator(); ExclusionIt; ++ExclusionIt)
                    {
                        if (const auto Exclusion = ExclusionIt->Get())
                        {
                            if (Exclusion->ExclusionMatches(*Object, Path))
                            {
                                Exclusions.Add(Exclusion);
                            }
                        }
                    }

                    for (auto RuleSetIt = Config->RuleSets.CreateIterator(); RuleSetIt; ++RuleSetIt)
                    {
                        if (const auto RuleSet = RuleSetIt->Get())
                        {
                            if (!ProcessRuleSetForObject(Config, RuleSet, Exclusions, Object, ProcessRuleFunction))
                            {
                                return;
                            }
                        }
                        else
                        {
                            UE_LOG(
                                RuleRanger,
                                Error,
                                TEXT("ProcessRule: Invalid RuleSet skipped when processing rules for %s in config %s"),
                                *Object->GetName(),
                                *Config->GetName());
                        }
                    }
                }
            }
            else
            {
                UE_LOG(RuleRanger,
                       Error,
                       TEXT("Invalid RuleSetConfig skipped when processing rules for %s"),
                       *Object->GetName());
            }
        }
    }

    // We need to check the context as there are cases when Object is not valid
    // on the first call through (i.e. another subsystem has already renamed/modified object or
    // it is a fbx with no animation or mesh data etc) and
    // thus ActionContext is not yet initialized
    if (IsValid(ActionContext))
    {
        ActionContext->ClearContext();
    }
}

bool URuleRangerEditorSubsystem::IsMatchingRulePresentForObject(URuleRangerConfig* const Config,
                                                                URuleRangerRuleSet* const RuleSet,
                                                                UObject* InObject,
                                                                const FRuleRangerRuleFn& ProcessRuleFunction)
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
                        "from config '%s' when analyzing object '%s'"),
                   RuleIndex,
                   *RuleSet->GetName(),
                   *Config->GetName(),
                   *InObject->GetName());
        }
        RuleIndex++;
    }
    return false;
}

bool URuleRangerEditorSubsystem::IsMatchingRulePresent(UObject* InObject, const FRuleRangerRuleFn& ProcessRuleFunction)
{
    if (IsValid(InObject))
    {
        auto Configs = GetCurrentRuleSetConfigs();
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("IsMatchingRulePresent: Located %d Rule Set Config(s) when discovering rules for object %s"),
               Configs.Num(),
               *InObject->GetName());
        for (auto ConfigIt = Configs.CreateIterator(); ConfigIt; ++ConfigIt)
        {
            if (const auto Config = ConfigIt->LoadSynchronous())
            {
                if (const auto Path = InObject->GetPathName(); Config->ConfigMatches(Path))
                {
                    for (auto RuleSetIt = Config->RuleSets.CreateIterator(); RuleSetIt; ++RuleSetIt)
                    {
                        if (const auto RuleSet = RuleSetIt->Get())
                        {
                            if (IsMatchingRulePresentForObject(Config, RuleSet, InObject, ProcessRuleFunction))
                            {
                                return true;
                            }
                            UE_LOG(RuleRanger,
                                   Error,
                                   TEXT("IsMatchingRulePresent: Invalid RuleSet skipped when processing "
                                        "rules for %s in config %s"),
                                   *InObject->GetName(),
                                   *Config->GetName());
                        }
                    }
                }
                else
                {
                    UE_LOG(RuleRanger,
                           Error,
                           TEXT("IsMatchingRulePresent: Invalid RuleSetConfig skipped when processing rules for %s"),
                           *InObject->GetName());
                }
            }
        }
    }
    return false;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
TArray<TSoftObjectPtr<URuleRangerConfig>> URuleRangerEditorSubsystem::GetCurrentRuleSetConfigs()
{
    const auto DeveloperSettings = GetMutableDefault<URuleRangerDeveloperSettings>();
    check(IsValid(DeveloperSettings));
    return DeveloperSettings->Configs;
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
        ActionContext->ResetContext(Rule, InObject, Trigger);

        Rule->Apply(ActionContext, InObject);

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
        if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
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
        ActionContext->ResetContext(Rule, InObject, ERuleRangerActionTrigger::AT_Validate);

        Rule->Apply(ActionContext, InObject);

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
        if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
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
        ActionContext->ResetContext(Rule, InObject, ERuleRangerActionTrigger::AT_Fix);

        Rule->Apply(ActionContext, InObject);

        ActionContext->EmitMessageLogs();
        // ReSharper disable once CppTooWideScopeInitStatement
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
        if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
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
