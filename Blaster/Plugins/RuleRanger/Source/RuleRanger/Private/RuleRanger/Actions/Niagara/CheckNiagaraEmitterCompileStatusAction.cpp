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

#include "CheckNiagaraEmitterCompileStatusAction.h"
#include "NiagaraScript.h"
#include "NiagaraScriptSource.h"

bool UCheckNiagaraEmitterCompileStatusAction::ValidateScript(URuleRangerActionContext* ActionContext,
                                                             const UObject* Object,
                                                             UNiagaraScript* Script) const
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
                    FString::Printf(TEXT("NiagaraEmitter status is BeingCreated for "
                                         "script %s. Status valid."),
                                    *Name));
            break;

        case ENiagaraScriptCompileStatus::NCS_Dirty:
            ActionContext->Error(FText::FromString(FString::Printf(TEXT("NiagaraEmitter status is dirty for "
                                                                        "script %s and needs to be recompiled"),
                                                                   *Name)));
            return false;

        case ENiagaraScriptCompileStatus::NCS_UpToDate:
            LogInfo(Object,
                    FString::Printf(TEXT("NiagaraEmitter status is UpToDate for "
                                         "script %s. Status valid."),
                                    *Name));
            break;
        case ENiagaraScriptCompileStatus::NCS_Error:
            ActionContext->Error(
                FText::FromString(FString::Printf(TEXT("NiagaraEmitter has an error status for "
                                                       "script %s. Fix errors and recompile NiagaraEmitter."),
                                                  *Name)));
            return false;
            ;
        case ENiagaraScriptCompileStatus::NCS_UpToDateWithWarnings:
        case ENiagaraScriptCompileStatus::NCS_ComputeUpToDateWithWarnings:
            if (bErrorOnUpToDateWithWarnings)
            {
                ActionContext->Error(FText::FromString(FString::Printf(TEXT("NiagaraEmitter is UpToDate but has "
                                                                            "warnings for script %s. Fix warnings and "
                                                                            "recompile NiagaraEmitter or set "
                                                                            "bErrorOnUpToDateWithWarnings=false"),
                                                                       *Name)));
                return false;
            }
            else
            {
                LogInfo(Object,
                        FString::Printf(TEXT("NiagaraEmitter status is UpToDate but has warnings "
                                             "for script %s and bErrorOnUpToDateWithWarnings=false. Status valid."),
                                        *Name));
            }
            break;
        case ENiagaraScriptCompileStatus::NCS_Unknown:
        default:
            if (bErrorOnUnknown)
            {
                ActionContext->Error(
                    FText::FromString(FString::Printf(TEXT("NiagaraEmitter has an Unknown status for script %s. "
                                                           "Recompile NiagaraEmitter or set bErrorOnUnknown to true"),
                                                      *Name)));
                return false;
            }
            else
            {
                LogInfo(Object,
                        FString::Printf(TEXT("NiagaraEmitter status is Unknown for script "
                                             "%s and bErrorOnUnknown=false. Status valid."),
                                        *Name));
            }
    }
    return true;
}

void UCheckNiagaraEmitterCompileStatusAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                   UObject* Object)
{
    const auto Emitter = CastChecked<UNiagaraEmitter>(Object);
    if (const auto& EmitterData = Emitter->GetLatestEmitterData())
    {
        TArray<UNiagaraScript*> Scripts;

        EmitterData->GetScripts(Scripts, true, true);
        for (const auto Script : Scripts)
        {
            if (!ValidateScript(ActionContext, Object, Script))
            {
                return;
            }
        }
    }
}

UClass* UCheckNiagaraEmitterCompileStatusAction::GetExpectedType()
{
    return UNiagaraEmitter::StaticClass();
}
