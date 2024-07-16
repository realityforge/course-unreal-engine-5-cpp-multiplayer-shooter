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

#include "EnsureNoDisabledEmittersAction.h"
#include "NiagaraSystem.h"

void UEnsureNoDisabledEmittersAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto System = CastChecked<UNiagaraSystem>(Object);
    for (const FNiagaraEmitterHandle& Handle : System->GetEmitterHandles())
    {
        if (!Handle.GetIsEnabled())
        {
            ActionContext->Error(
                FText::FromString(FString::Printf(TEXT("NiagaraSystem has a disabled emitter named %s. "
                                                       "Disabled emitters are loaded at runtime and should "
                                                       "be removed unless they are dynamically enabled at "
                                                       "runtime. Either remove the disabled emitter or exclude "
                                                       "this NiagaraSystem from the RulrRanger rule."),
                                                  *Handle.GetName().ToString())));
        }
    }
}

UClass* UEnsureNoDisabledEmittersAction::GetExpectedType()
{
    return UNiagaraSystem::StaticClass();
}
