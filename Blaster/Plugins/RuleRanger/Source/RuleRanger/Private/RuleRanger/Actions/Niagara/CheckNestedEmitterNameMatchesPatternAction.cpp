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

#include "CheckNestedEmitterNameMatchesPatternAction.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleEmitter.h"

void UCheckNestedEmitterNameMatchesPatternAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                       UObject* Object)
{
    const UParticleSystem* ParticleSystem = CastChecked<UParticleSystem>(Object);

    const FRegexPattern RegexPattern(Pattern,
                                     bCaseSensitive ? ERegexPatternFlags::None : ERegexPatternFlags::CaseInsensitive);
    FRegexMatcher ElementRegexMatcher(RegexPattern, Object->GetName());

    for (const auto& Emitter : ParticleSystem->Emitters)
    {
        const auto& EmitterName = Emitter->EmitterName.ToString();

        if (!ElementRegexMatcher.FindNext())
        {
            const auto& ErrorMessage =
                FString::Printf(TEXT("Niagara System contains an emitter named '%s' that does not match the regex %s."),
                                *EmitterName,
                                *Pattern);
            ActionContext->Error(FText::FromString(ErrorMessage));
        }
    }
}

UClass* UCheckNestedEmitterNameMatchesPatternAction::GetExpectedType()
{
    return UNiagaraSystem::StaticClass();
}
