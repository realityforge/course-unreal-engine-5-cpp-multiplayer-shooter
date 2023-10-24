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

#include "RuleRanger/Actions/CheckTextureResolutionCompressibleAction.h"
#include "Editor.h"
#include "RuleRangerLogging.h"

void UCheckTextureResolutionCompressibleAction::Apply_Implementation(
    TScriptInterface<IRuleRangerActionContext>& ActionContext,
    UObject* Object)
{
    if (IsValid(Object))
    {
        if (const auto Texture = Cast<UTexture2D>(Object); !Texture)
        {
            UE_LOG(RuleRanger,
                   Error,
                   TEXT("CheckTextureResolutionCompressibleAction: Attempt to run on Object %s "
                        "that is not a Texture2D instance."),
                   *Object->GetName());
        }
        else
        {
            const int Divisor = ETextureDivisor::TD_Four == TextureDivisor ? 4
                : ETextureDivisor::TD_Eight == TextureDivisor              ? 8
                                                                           : 12;
            const int32 SizeX = Texture->GetSizeX();
            const int32 SizeY = Texture->GetSizeY();

            const bool bInvalidX = 0 != (SizeX % Divisor);
            const bool bInvalidY = 0 != (SizeY % Divisor);

            if (bInvalidX || bInvalidY)
            {
                FFormatNamedArguments Arguments;
                Arguments.Add(TEXT("X"), FText::FromString(FString::FromInt(SizeX)));
                Arguments.Add(TEXT("Y"), FText::FromString(FString::FromInt(SizeY)));
                Arguments.Add(TEXT("Divisor"), FText::FromString(FString::FromInt(Divisor)));

                if (bInvalidX && bInvalidY)
                {
                    const FText Message =
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "CheckTextureResolutionCompressibleAction_FailXY",
                                                "Texture has dimensions {X}x{Y} and neither width nor"
                                                " height is divisible by {Divisor}. Fix both dimensions"),
                                      Arguments);
                    ActionContext->Error(Message);
                }
                else if (bInvalidX)
                {
                    const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                                  "CheckTextureResolutionCompressibleAction_FailX",
                                                                  "Texture has dimensions {X}x{Y} and width is not "
                                                                  "divisible by {Divisor}. Fix the width dimension"),
                                                        Arguments);
                    ActionContext->Error(Message);
                }
                else
                {
                    const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                                  "CheckTextureResolutionCompressibleAction_FailX",
                                                                  "Texture has dimensions {X}x{Y} and height is not "
                                                                  "divisible by {Divisor}. Fix the height dimension"),
                                                        Arguments);
                    ActionContext->Error(Message);
                }
            }
            else
            {
                UE_LOG(RuleRanger,
                       Verbose,
                       TEXT("CheckTextureResolutionCompressibleAction(%s): Texture dimensions are divisible by %d. "
                            "No action required"),
                       *Object->GetName(),
                       Divisor);
            }
        }
    }
}
