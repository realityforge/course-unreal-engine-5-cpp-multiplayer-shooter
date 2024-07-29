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

#include "CheckNiagaraSystemCompileStatusAction.h"
#include "NiagaraScript.h"
#include "NiagaraScriptSource.h"
#include "NiagaraSystem.h"

bool UCheckNiagaraSystemCompileStatusAction::ValidateScript(URuleRangerActionContext* ActionContext,
                                                            const UObject* Object,
                                                            const FString& ContainerContext,
                                                            UNiagaraScript* const Script) const
{
    const auto VMExecutableData = Script->GetVMExecutableData();
    const auto Name = Script->GetName();
    // ReSharper disable once CppTooWideScope
    const auto CompileStatus =
        VMExecutableData.IsValid() ? VMExecutableData.LastCompileStatus : Script->GetLastCompileStatus();
    switch (CompileStatus)
    {
        case ENiagaraScriptCompileStatus::NCS_BeingCreated:
            LogInfo(Object,
                    FString::Printf(TEXT("NiagaraSystem status is BeingCreated for "
                                         "script %s in %s. Status valid."),
                                    *Name,
                                    *ContainerContext));
            break;

        case ENiagaraScriptCompileStatus::NCS_Dirty:
            ActionContext->Error(FText::FromString(FString::Printf(TEXT("NiagaraSystem status is dirty for "
                                                                        "script %s in %s and needs to be recompiled"),
                                                                   *Name,
                                                                   *ContainerContext)));
            return false;

        case ENiagaraScriptCompileStatus::NCS_UpToDate:
            LogInfo(Object,
                    FString::Printf(TEXT("NiagaraSystem status is UpToDate for "
                                         "script %s in %s. Status valid."),
                                    *Name,
                                    *ContainerContext));
            break;
        case ENiagaraScriptCompileStatus::NCS_Error:
            ActionContext->Error(
                FText::FromString(FString::Printf(TEXT("NiagaraSystem has an error status for "
                                                       "script %s in %s. Fix errors and recompile NiagaraSystem."),
                                                  *Name,
                                                  *ContainerContext)));
            return false;

        case ENiagaraScriptCompileStatus::NCS_UpToDateWithWarnings:
        case ENiagaraScriptCompileStatus::NCS_ComputeUpToDateWithWarnings:
            if (bErrorOnUpToDateWithWarnings)
            {
                ActionContext->Error(
                    FText::FromString(FString::Printf(TEXT("NiagaraSystem is UpToDate but has "
                                                           "warnings for script %s in %s. Fix warnings and "
                                                           "recompile NiagaraSystem or set "
                                                           "bErrorOnUpToDateWithWarnings=false"),
                                                      *Name,
                                                      *ContainerContext)));
                return false;
            }
            else
            {
                LogInfo(
                    Object,
                    FString::Printf(TEXT("NiagaraSystem status is UpToDate but has warnings "
                                         "for script %s in %s and bErrorOnUpToDateWithWarnings=false. Status valid."),
                                    *Name,
                                    *ContainerContext));
            }

            break;
        case ENiagaraScriptCompileStatus::NCS_Unknown:
        default:
            if (bErrorOnUnknown)
            {
                ActionContext->Error(
                    FText::FromString(FString::Printf(TEXT("NiagaraSystem has an Unknown status for script %s in %s. "
                                                           "Recompile NiagaraSystem or set bErrorOnUnknown to true"),
                                                      *Name,
                                                      *ContainerContext)));
                return false;
            }
            else
            {
                LogInfo(Object,
                        FString::Printf(TEXT("NiagaraSystem status is Unknown for script "
                                             "%s in %s and bErrorOnUnknown=false. Status valid."),
                                        *Name,
                                        *ContainerContext));
            }
    }
    return true;
}

void UCheckNiagaraSystemCompileStatusAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                  UObject* Object)
{
    const auto System = CastChecked<UNiagaraSystem>(Object);

    if (System->GetSystemSpawnScript()->IsCompilable())
    {
        if (!ValidateScript(ActionContext, Object, TEXT("the NiagaraSystem"), System->GetSystemSpawnScript()))
        {
            //            return;
        }
    }
    if (System->GetSystemUpdateScript()->IsCompilable())
    {
        if (!ValidateScript(ActionContext, Object, TEXT("the NiagaraSystem"), System->GetSystemUpdateScript()))
        {
            return;
        }
    }
    if (System->NeedsRequestCompile())
    {
        // If the system needs a recompile and we attempt to get status, we will get an Unknown Status.
        // This code just makes sure that any pending compilation request has completed and thus when we are running
        // the rule the system is correctly prepared.
        if (!System->HasOutstandingCompilationRequests(true))
        {
            LogInfo(Object, FString::Printf(TEXT("NiagaraSystem has NeedsRequestCompile set, Requesting compile...")));
            System->RequestCompile(false);
        }
        else
        {
            LogInfo(
                Object,
                FString::Printf(TEXT(
                    "NiagaraSystem has NeedsRequestCompile set but request already pending. Waiting for compilation to complete...")));
        }
        System->WaitForCompilationComplete(true);
        LogInfo(Object, FString::Printf(TEXT("NiagaraSystem compilation complete.")));
    }
    for (const FNiagaraEmitterHandle& Handle : System->GetEmitterHandles())
    {
        if (const auto& EmitterData = Handle.GetEmitterData())
        {
            if (Handle.GetIsEnabled())
            {
                TArray<UNiagaraScript*> Scripts;

                EmitterData->GetScripts(Scripts, true, true);
                const auto Context = FString::Printf(TEXT("the Emitter named %s"), *Handle.GetName().ToString());
                for (const auto Script : Scripts)
                {
                    if (!ValidateScript(ActionContext, Object, Context, Script))
                    {
                        return;
                    }
                }
            }
        }
    }
}

UClass* UCheckNiagaraSystemCompileStatusAction::GetExpectedType()
{
    return UNiagaraSystem::StaticClass();
}
