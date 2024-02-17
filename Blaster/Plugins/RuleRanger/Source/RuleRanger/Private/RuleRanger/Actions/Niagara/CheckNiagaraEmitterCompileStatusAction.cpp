﻿/*
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

void UCheckNiagaraEmitterCompileStatusAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                   UObject* Object)
{
    const auto Emitter = CastChecked<UNiagaraEmitter>(Object);
    TArray<UNiagaraScript*> Scripts;
    if (const auto& EmitterData = Emitter->GetLatestEmitterData())
    {
        Scripts.Add(EmitterData->EmitterSpawnScriptProps.Script);
        Scripts.Add(EmitterData->EmitterUpdateScriptProps.Script);
        Scripts.Add(EmitterData->SpawnScriptProps.Script);
        Scripts.Add(EmitterData->UpdateScriptProps.Script);
        if (ENiagaraSimTarget::CPUSim == EmitterData->SimTarget)
        {
            for (const auto& ScriptProp : EmitterData->EventHandlerScriptProps)
            {
                Scripts.Add(ScriptProp.Script);
            }
        }
        if (ENiagaraSimTarget::GPUComputeSim == EmitterData->SimTarget)
        {
            Scripts.Add(EmitterData->GetGPUComputeScript());
        }
    }

    for (const auto Script : Scripts)
    {
        if (Script && Script->IsCompilable())
        {
            const auto VMExecutableData = Script->GetVMExecutableData();

            // ReSharper disable once CppTooWideScope
            const auto CompileStatus =
                VMExecutableData.IsValid() ? VMExecutableData.LastCompileStatus : Script->GetLastCompileStatus();
            switch (CompileStatus)
            {
                case ENiagaraScriptCompileStatus::NCS_BeingCreated:
                    LogInfo(Object, TEXT("NiagaraEmitter status is BeingCreated. Status valid."));
                    break;

                case ENiagaraScriptCompileStatus::NCS_Dirty:
                    ActionContext->Error(FText::FromString("NiagaraEmitter is dirty and needs to be recompiled"));
                    return;

                case ENiagaraScriptCompileStatus::NCS_UpToDate:
                    LogInfo(Object, TEXT("NiagaraEmitter status is UpToDate. Status valid."));
                    break;

                case ENiagaraScriptCompileStatus::NCS_Error:
                    ActionContext->Error(FText::FromString(
                        "NiagaraEmitter has an error status. Fix error and recompile NiagaraEmitter."));
                    return;

                case ENiagaraScriptCompileStatus::NCS_UpToDateWithWarnings:
                case ENiagaraScriptCompileStatus::NCS_ComputeUpToDateWithWarnings:
                    if (bErrorOnUpToDateWithWarnings)
                    {
                        ActionContext->Error(
                            FText::FromString("NiagaraEmitter is UpToDate but has warnings. Fix warnings "
                                              "and recompile or set bErrorOnUpToDateWithWarnings=false"));
                        return;
                    }
                    else
                    {
                        LogInfo(
                            Object,
                            TEXT(
                                "NiagaraEmitter status is Unknown and bErrorOnUpToDateWithWarnings=false. Status valid."));
                    }

                    break;
                case ENiagaraScriptCompileStatus::NCS_Unknown:
                default:
                    if (bErrorOnUnknown)
                    {
                        ActionContext->Error(FText::FromString(
                            "NiagaraEmitter has an Unknown status. Recompile NiagaraEmitter or set bErrorOnUnknown to true"));
                        return;
                    }
                    else
                    {
                        LogInfo(Object,
                                TEXT("NiagaraEmitter status is Unknown and bErrorOnUnknown=false. Status valid."));
                    }
            }
        }
    }
}

UClass* UCheckNiagaraEmitterCompileStatusAction::GetExpectedType()
{
    return UNiagaraEmitter::StaticClass();
}
