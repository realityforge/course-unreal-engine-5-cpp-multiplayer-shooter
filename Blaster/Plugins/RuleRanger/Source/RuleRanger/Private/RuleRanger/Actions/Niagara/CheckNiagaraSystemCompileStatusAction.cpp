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

void UCheckNiagaraSystemCompileStatusAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                  UObject* Object)
{
    const auto System = CastChecked<UNiagaraSystem>(Object);
    TArray<UNiagaraScript*> Scripts;
    Scripts.Add(System->GetSystemSpawnScript());
    Scripts.Add(System->GetSystemUpdateScript());

    for (const FNiagaraEmitterHandle& Handle : System->GetEmitterHandles())
    {
        if (const auto& EmitterData = Handle.GetEmitterData())
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
    }

    for (const auto Script : Scripts)
    {
        if (Script)
        {
            switch (Script->GetLastCompileStatus())
            {
                case ENiagaraScriptCompileStatus::NCS_BeingCreated:
                    LogInfo(Object, TEXT("NiagaraSystem status is BeingCreated. Status valid."));
                    break;

                case ENiagaraScriptCompileStatus::NCS_Dirty:
                    ActionContext->Error(FText::FromString("NiagaraSystem is dirty and needs to be recompiled"));
                    return;

                case ENiagaraScriptCompileStatus::NCS_UpToDate:
                    LogInfo(Object, TEXT("NiagaraSystem status is UpToDate. Status valid."));
                    break;
                case ENiagaraScriptCompileStatus::NCS_Error:
                    ActionContext->Error(FText::FromString(
                        "NiagaraSystem has an Unknown status. Fix error and recompile NiagaraSystem."));
                    return;
                    ;
                case ENiagaraScriptCompileStatus::NCS_UpToDateWithWarnings:
                case ENiagaraScriptCompileStatus::NCS_ComputeUpToDateWithWarnings:
                    if (bErrorOnUpToDateWithWarnings)
                    {
                        ActionContext->Error(
                            FText::FromString("NiagaraSystem is UpToDate but has warnings. Fix warnings "
                                              "and recompile or set bErrorOnUpToDateWithWarnings=false"));
                        return;
                    }
                    else
                    {
                        LogInfo(
                            Object,
                            TEXT(
                                "NiagaraSystem status is Unknown and bErrorOnUpToDateWithWarnings=false. Status valid."));
                    }

                    break;
                case ENiagaraScriptCompileStatus::NCS_Unknown:
                default:
                    if (bErrorOnUnknown)
                    {
                        ActionContext->Error(
                            FText::FromString("NiagaraSystem has an Unknown status. Recompile NiagaraSystem"));
                        return;
                    }
                    else
                    {
                        LogInfo(Object,
                                TEXT("NiagaraSystem status is Unknown and bErrorOnUnknown=false. Status valid."));
                    }
            }
        }
    }
}

UClass* UCheckNiagaraSystemCompileStatusAction::GetExpectedType()
{
    return UNiagaraSystem::StaticClass();
}
