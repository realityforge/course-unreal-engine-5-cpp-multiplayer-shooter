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
    if (IsValid(Object))
    {
        if (!ActionContext)
        {
            UE_LOG(RuleRanger, Verbose, TEXT("RuleRangerEditorSubsystem: Creating the initial ActionContext"));
            ActionContext = NewObject<UActionContextImpl>(this, UActionContextImpl::StaticClass());
        }

        const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();

        // Use a metadata tag when we have imported an asset so that when we try to reimport asset we can
        // identify this through the presence of tag.
        const bool bIsReimport = Subsystem && Subsystem->GetMetadataTag(Object, ImportMarkerKey) == ImportMarkerValue;
        const auto DeveloperSettings = GetMutableDefault<URuleRangerDeveloperSettings>();
        check(IsValid(DeveloperSettings));
        UE_LOG(RuleRanger,
               Verbose,
               TEXT("OnAssetPostImport(%s) discovered %d Rule Set(s)"),
               *Object->GetName(),
               DeveloperSettings->Rules.Num());
        for (auto RuleSetIt = DeveloperSettings->Rules.CreateIterator(); RuleSetIt; ++RuleSetIt)
        {
            const auto RuleSet = RuleSetIt->LoadSynchronous();
            if (const auto Path = Object->GetPathName(); Path.StartsWith(RuleSet->Dir.Path))
            {
                UE_LOG(RuleRanger,
                       Verbose,
                       TEXT("OnAssetPostImport(%s) processing Rule Set %s"),
                       *Object->GetName(),
                       *RuleSet->GetName());
                for (const auto RulePtr : RuleSet->Rules)
                {
                    // ReSharper disable once CppTooWideScopeInitStatement
                    const auto Rule = RulePtr.Get();
                    if ((!bIsReimport && Rule->bApplyOnImport) || (bIsReimport && Rule->bApplyOnReimport))
                    {
                        UE_LOG(RuleRanger,
                               Verbose,
                               TEXT("OnAssetPostImport(%s) applying rule %s during %s."),
                               *Object->GetName(),
                               *Rule->GetName(),
                               bIsReimport ? TEXT("reimport") : TEXT("import"));
                        const ERuleRangerActionTrigger Trigger =
                            bIsReimport ? ERuleRangerActionTrigger::AT_Reimport : ERuleRangerActionTrigger::AT_Import;
                        ActionContext->ResetContext(Object, Trigger);

                        TScriptInterface<IRuleRangerActionContext> ScriptInterfaceActionContext(ActionContext);
                        Rule->Apply(ScriptInterfaceActionContext, Object);

                        ActionContext->EmitMessageLogs();
                        const auto State = ActionContext->GetState();
                        if (ERuleRangerActionState::AS_Fatal == State)
                        {
                            UE_LOG(RuleRanger,
                                   Verbose,
                                   TEXT("OnAssetPostImport(%s) applied rule %s which resulted in fatal error. "
                                        "Processing rules will not continue."),
                                   *Object->GetName(),
                                   *Rule->GetName());
                            ActionContext->ClearContext();
                            return;
                        }
                        else if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
                        {
                            UE_LOG(RuleRanger,
                                   Verbose,
                                   TEXT("OnAssetPostImport(%s) applied rule %s which resulted in error. "
                                        "Processing rules will not continue as ContinueOnError=False."),
                                   *Object->GetName(),
                                   *Rule->GetName());
                            ActionContext->ClearContext();
                            return;
                        }
                    }
                    else
                    {
                        UE_LOG(RuleRanger,
                               Verbose,
                               TEXT("OnAssetPostImport(%s) skipped rule %s as flag on "
                                    "rule does not enable rule during %s."),
                               *Object->GetName(),
                               *Rule->GetName(),
                               bIsReimport ? TEXT("reimport") : TEXT("import"));
                    }
                }
            }
            else
            {
                UE_LOG(RuleRanger,
                       Verbose,
                       TEXT("OnAssetPostImport(%s) skipped processing Rule Set %s as the rule set restricted to "
                            "content within %s but content path was %s"),
                       *Object->GetName(),
                       *RuleSet->GetName(),
                       *RuleSet->Dir.Path,
                       *Path);
            }
        }
        ActionContext->ClearContext();
    }
}
