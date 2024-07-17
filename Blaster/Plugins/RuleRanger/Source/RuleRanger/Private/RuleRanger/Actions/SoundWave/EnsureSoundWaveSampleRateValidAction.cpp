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

#include "EnsureSoundWaveSampleRateValidAction.h"

UEnsureSoundWaveSampleRateValidAction::UEnsureSoundWaveSampleRateValidAction()
{
    ValidSampleRates.Add(48000);
}

void UEnsureSoundWaveSampleRateValidAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                 UObject* Object)
{
    const USoundWave* SoundWave = CastChecked<USoundWave>(Object);

    // ReSharper disable once CppTooWideScopeInitStatement
    const float SampleRate = SoundWave->GetSampleRateForCurrentPlatform();
    if (!ValidSampleRates.Contains((int32)SampleRate))
    {
        FString ValidValues{ "" };
        for (const int32 ValidSampleRate : ValidSampleRates)
        {
            if (0 != ValidValues.Len())
            {
                ValidValues.Append(", ");
            }
            ValidValues.Appendf(TEXT("%d"), ValidSampleRate);
        }

        const auto& ErrorMessage = FString::Printf(TEXT("SoundWave has a sample rate of %f but must have one of: %s"),
                                                   SampleRate,
                                                   *ValidValues);
        ActionContext->Error(FText::FromString(ErrorMessage));
    }
}

UClass* UEnsureSoundWaveSampleRateValidAction::GetExpectedType()
{
    return USoundWave::StaticClass();
}
