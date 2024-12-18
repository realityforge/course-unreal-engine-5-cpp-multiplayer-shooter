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

#include UE_INLINE_GENERATED_CPP_BY_NAME(RuleRangerEditorSubsystem)

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
    ProcessRule(
        InObject,
        [this](URuleRangerConfig* const Config,
               URuleRangerRuleSet* const RuleSet,
               URuleRangerRule* Rule,
               UObject* InnerInObject) mutable { return ProcessDemandScan(Config, RuleSet, Rule, InnerInObject); });
}

void URuleRangerEditorSubsystem::ScanAndFixObject(UObject* InObject)
{
    ProcessRule(InObject,
                [this](URuleRangerConfig* const Config,
                       URuleRangerRuleSet* const RuleSet,
                       URuleRangerRule* Rule,
                       UObject* InnerInObject) mutable {
                    return ProcessDemandScanAndFix(Config, RuleSet, Rule, InnerInObject);
                });
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerEditorSubsystem::OnAssetPostImport([[maybe_unused]] UFactory* Factory, UObject* Object)
{
    const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();

    const static FName NAME_ImportMarkerKey = FName(TEXT("RuleRanger.ImportProcessed"));
    // Use a metadata tag when we have imported an asset so that when we try to reimport asset, we can
    // identify this through the presence of tag.
    const bool bIsReimport = Subsystem && Subsystem->GetMetadataTag(Object, NAME_ImportMarkerKey) == ImportMarkerValue;

    ProcessRule(Object,
                [this, bIsReimport](URuleRangerConfig* const Config,
                                    URuleRangerRuleSet* const RuleSet,
                                    URuleRangerRule* Rule,
                                    UObject* InObject) {
                    return ProcessOnAssetPostImportRule(Config, RuleSet, Rule, bIsReimport, InObject);
                });
}

bool URuleRangerEditorSubsystem::ProcessRuleSetForObject(URuleRangerConfig* const Config,
                                                         URuleRangerRuleSet* const RuleSet,
                                                         TArray<URuleRangerRuleExclusion*> Exclusions,
                                                         UObject* Object,
                                                         const FRuleRangerRuleFn& ProcessRuleFunction)
{
    RR_VERY_VERBOSE_ALOG("ProcessRule: Processing Rule Set %s for object %s", *RuleSet->GetName(), *Object->GetName());

    for (auto ExclusionIt = Exclusions.CreateIterator(); ExclusionIt; ++ExclusionIt)
    {
        if (const auto Exclusion = *ExclusionIt)
        {
            if (Exclusion->RuleSets.Contains(RuleSet))
            {
                RR_VERY_VERBOSE_ALOG(
                    "ProcessRule: Rule Set %s excluded for object %s due to exclusion rule. Reason: %s",
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
            RR_VERY_VERBOSE_ALOG("ProcessRule: Processing Nested Rule Set %s for object %s",
                                 *NestedRuleSet->GetName(),
                                 *Object->GetName());
            if (!ProcessRuleSetForObject(Config, NestedRuleSet, Exclusions, Object, ProcessRuleFunction))
            {
                return false;
            }
            RR_VERY_VERBOSE_ALOG("ProcessRule: Completed processing of Nested Rule Set %s for object %s",
                                 *NestedRuleSet->GetName(),
                                 *Object->GetName());
        }
        else
        {
            RR_ERROR_ALOG("ProcessRule: Invalid RuleSet skipped when processing rules for %s in config %s",
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
            bool bSkipRule = false;
            for (auto ExclusionIt = Exclusions.CreateIterator(); ExclusionIt; ++ExclusionIt)
            {
                if (const auto Exclusion = *ExclusionIt)
                {
                    if (Exclusion->Rules.Contains(Rule))
                    {
                        RR_VERY_VERBOSE_ALOG("ProcessRule: Rule %s from RuleSet %s was excluded for "
                                             "object %s due to exclusion rule. Reason: %s",
                                             *Rule->GetName(),
                                             *RuleSet->GetName(),
                                             *Object->GetName(),
                                             *Exclusion->Description.ToString());
                        bSkipRule = true;
                    }
                }
            }

            if (!bSkipRule && !ProcessRuleFunction(Config, RuleSet, Rule, Object))
            {
                RR_VERY_VERBOSE_ALOG(
                    "ProcessRule: Rule %s from RuleSet %s indicated that following rules should be skipped for %s",
                    *Rule->GetName(),
                    *RuleSet->GetName(),
                    *Object->GetName());
                ActionContext->ClearContext();
                return false;
            }
        }
        else
        {
            RR_ERROR_ALOG("ProcessRule: Invalid Rule skipped at index %d in rule set '%s' "
                          "from config '%s' when analyzing object '%s'",
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
            RR_VERY_VERBOSE_ALOG("RuleRangerEditorSubsystem: Creating the initial ActionContext");
            ActionContext = NewObject<URuleRangerActionContext>(this, URuleRangerActionContext::StaticClass());
        }

        auto Configs = GetCurrentRuleSetConfigs();
        const auto Path = Object->GetPathName();
        RR_VERY_VERBOSE_ALOG("ProcessRule: Located %d Rule Set Config(s) when discovering rules for object %s at %s",
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
                            RR_ERROR_ALOG(
                                "ProcessRule: Invalid RuleSet skipped when processing rules for %s in config %s",
                                *Object->GetName(),
                                *Config->GetName());
                        }
                    }
                }
            }
            else
            {
                RR_ERROR_ALOG("Invalid RuleSetConfig skipped when processing rules for %s", *Object->GetName());
            }
        }
    }

    // We need to check that the ActionContext is valid as it may not have been initialized.
    // This happens when Object is not valid, gets renamed or removed by an action or we are importing
    // from an FBX that has no animation or mesh data.
    if (IsValid(ActionContext))
    {
        ActionContext->ClearContext();
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool URuleRangerEditorSubsystem::IsMatchingRulePresentForObject(URuleRangerConfig* const Config,
                                                                URuleRangerRuleSet* const RuleSet,
                                                                UObject* InObject,
                                                                const FRuleRangerRuleFn& ProcessRuleFunction)
{
    RR_VERY_VERBOSE_ALOG("IsMatchingRulePresent: Processing Rule Set %s for object %s",
                         *RuleSet->GetName(),
                         *InObject->GetName());

    int RuleSetIndex = 0;
    for (auto RuleSetIt = RuleSet->RuleSets.CreateIterator(); RuleSetIt; ++RuleSetIt)
    {
        if (const auto ChildRuleSet = RuleSetIt->Get())
        {
            if (IsMatchingRulePresentForObject(Config, ChildRuleSet, InObject, ProcessRuleFunction))
            {
                return true;
            }
        }
        else
        {
            RR_ERROR_ALOG("IsMatchingRulePresentForObject: Invalid RuleSet skipped at index %d "
                          "processing child rulesets of ruleset named %s for object %s",
                          RuleSetIndex,
                          *RuleSet->GetName(),
                          *InObject->GetName());
        }
        RuleSetIndex++;
    }

    int RuleIndex = 0;
    for (const auto RulePtr : RuleSet->Rules)
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        if (const auto Rule = RulePtr.Get(); IsValid(Rule))
        {
            if (ProcessRuleFunction(Config, RuleSet, Rule, InObject))
            {
                return true;
            }
        }
        else
        {
            RR_ERROR_ALOG("IsMatchingRulePresent: Invalid Rule skipped at index %d in "
                          "rule set '%s' "
                          "from config '%s' when analyzing object '%s'",
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
        RR_VERY_VERBOSE_ALOG(
            "IsMatchingRulePresent: Located %d Rule Set Config(s) when discovering rules for object %s",
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
                        }
                        else
                        {
                            RR_ERROR_ALOG("IsMatchingRulePresent: Invalid RuleSet skipped when processing "
                                          "rules for %s in config %s",
                                          *InObject->GetName(),
                                          *Config->GetName());
                        }
                    }
                }
            }
            else
            {
                RR_ERROR_ALOG("IsMatchingRulePresent: Invalid RuleSetConfig skipped when processing rules for %s",
                              *InObject->GetName());
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

bool URuleRangerEditorSubsystem::ProcessOnAssetPostImportRule(URuleRangerConfig* const Config,
                                                              URuleRangerRuleSet* const RuleSet,
                                                              URuleRangerRule* Rule,
                                                              const bool bIsReimport,
                                                              UObject* InObject) const
{
    check(ActionContext);

    if ((!bIsReimport && Rule->bApplyOnImport) || (bIsReimport && Rule->bApplyOnReimport))
    {
        RR_VERY_VERBOSE_ALOG("OnAssetPostImport(%s) applying rule %s during %s.",
                             *InObject->GetName(),
                             *Rule->GetName(),
                             bIsReimport ? TEXT("reimport") : TEXT("import"));
        const ERuleRangerActionTrigger Trigger =
            bIsReimport ? ERuleRangerActionTrigger::AT_Reimport : ERuleRangerActionTrigger::AT_Import;
        ActionContext->ResetContext(Config, RuleSet, Rule, InObject, Trigger);

        Rule->Apply(ActionContext, InObject);

        ActionContext->EmitMessageLogs();
        const auto State = ActionContext->GetState();
        if (ERuleRangerActionState::AS_Fatal == State)
        {
            RR_VERY_VERBOSE_ALOG("OnAssetPostImport(%s) applied rule %s which resulted in fatal error. "
                                 "Processing rules will not continue.",
                                 *InObject->GetName(),
                                 *Rule->GetName());
            ActionContext->ClearContext();
            return false;
        }
        if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
        {
            RR_VERY_VERBOSE_ALOG("OnAssetPostImport(%s) applied rule %s which resulted in error. "
                                 "Processing rules will not continue as ContinueOnError=False.",
                                 *InObject->GetName(),
                                 *Rule->GetName());
            ActionContext->ClearContext();
            return false;
        }
    }
    else
    {
        RR_VERY_VERBOSE_ALOG("OnAssetPostImport(%s) skipped rule %s as flag on "
                             "rule does not enable rule during %s.",
                             *InObject->GetName(),
                             *Rule->GetName(),
                             bIsReimport ? TEXT("reimport") : TEXT("import"));
    }
    return true;
}

bool URuleRangerEditorSubsystem::ProcessDemandScan(URuleRangerConfig* const Config,
                                                   URuleRangerRuleSet* const RuleSet,
                                                   URuleRangerRule* Rule,
                                                   UObject* InObject) const
{
    check(ActionContext);

    if (Rule->bApplyOnDemand)
    {
        RR_VERY_VERBOSE_ALOG("ProcessDemandScan(%s) applying rule %s.", *InObject->GetName(), *Rule->GetName());
        ActionContext->ResetContext(Config, RuleSet, Rule, InObject, ERuleRangerActionTrigger::AT_Validate);

        Rule->Apply(ActionContext, InObject);

        ActionContext->EmitMessageLogs();
        const auto State = ActionContext->GetState();
        if (ERuleRangerActionState::AS_Fatal == State)
        {
            RR_VERY_VERBOSE_ALOG("ProcessDemandScan(%s) applied rule %s which resulted in fatal error. "
                                 "Processing rules will not continue.",
                                 *InObject->GetName(),
                                 *Rule->GetName());
            ActionContext->ClearContext();
            return false;
        }
        if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
        {
            RR_VERY_VERBOSE_ALOG("ProcessDemandScan(%s) applied rule %s which resulted in error. "
                                 "Processing rules will not continue as ContinueOnError=False.",
                                 *InObject->GetName(),
                                 *Rule->GetName());
            ActionContext->ClearContext();
            return false;
        }
    }
    else
    {
        RR_VERY_VERBOSE_ALOG("ProcessDemandScan(%s) skipped rule %s as flag on "
                             "rule does not enable rule on demand.",
                             *InObject->GetName(),
                             *Rule->GetName());
    }
    return true;
}

bool URuleRangerEditorSubsystem::ProcessDemandScanAndFix(URuleRangerConfig* const Config,
                                                         URuleRangerRuleSet* const RuleSet,
                                                         URuleRangerRule* Rule,
                                                         UObject* InObject) const
{
    check(ActionContext);

    if (Rule->bApplyOnDemand)
    {
        RR_VERY_VERBOSE_ALOG("ProcessDemandScanAndFix(%s) applying rule %s.", *InObject->GetName(), *Rule->GetName());
        ActionContext->ResetContext(Config, RuleSet, Rule, InObject, ERuleRangerActionTrigger::AT_Fix);

        Rule->Apply(ActionContext, InObject);

        ActionContext->EmitMessageLogs();
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto State = ActionContext->GetState();
        if (ERuleRangerActionState::AS_Fatal == State)
        {
            RR_VERY_VERBOSE_ALOG("ProcessDemandScanAndFix(%s) applied rule %s which resulted in fatal error. "
                                 "Processing rules will not continue.",
                                 *InObject->GetName(),
                                 *Rule->GetName());
            ActionContext->ClearContext();
            return false;
        }
        if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
        {
            RR_VERY_VERBOSE_ALOG("ProcessDemandScanAndFix(%s) applied rule %s which resulted in error. "
                                 "Processing rules will not continue as ContinueOnError=False.",
                                 *InObject->GetName(),
                                 *Rule->GetName());
            ActionContext->ClearContext();
            return false;
        }
    }
    else
    {
        RR_VERY_VERBOSE_ALOG("ProcessDemandScanAndFix(%s) skipped rule %s as flag on "
                             "rule does not enable rule on demand.",
                             *InObject->GetName(),
                             *Rule->GetName());
    }
    return true;
}
