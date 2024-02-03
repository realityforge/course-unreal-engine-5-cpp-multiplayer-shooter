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

#include "EnsureTextureResolutionConstraintsAction.h"

void UEnsureTextureResolutionConstraintsAction::CheckPowerOfTwo(URuleRangerActionContext* ActionContext,
                                                                const UTexture2D* const Texture) const
{
    const int32 SizeX = Texture->GetSizeX();
    const int32 SizeY = Texture->GetSizeY();

    const bool bInvalidX = (0 != (SizeX & SizeX - 1));
    const bool bInvalidY = (0 != (SizeY & SizeY - 1));

    if (bInvalidX || bInvalidY)
    {
        FFormatNamedArguments Arguments;
        Arguments.Add(TEXT("X"), FText::FromString(FString::FromInt(SizeX)));
        Arguments.Add(TEXT("Y"), FText::FromString(FString::FromInt(SizeY)));

        if (bInvalidX && bInvalidY)
        {
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "EnsureTextureResolutionConstraintsAction_Pow2FailXY",
                                                          "Texture has dimensions {X}x{Y} and neither width nor"
                                                          " height is a power of two. Fix both dimensions"),
                                                Arguments);
            ActionContext->Error(Message);
        }
        else if (bInvalidX)
        {
            const FText Message = FText::Format(
                NSLOCTEXT("RuleRanger",
                          "EnsureTextureResolutionConstraintsAction_Pow2FailX",
                          "Texture has dimensions {X}x{Y} and width is not a power of two. Fix the width dimension"),
                Arguments);
            ActionContext->Error(Message);
        }
        else
        {
            const FText Message = FText::Format(
                NSLOCTEXT("RuleRanger",
                          "EnsureTextureResolutionConstraintsAction_Pow2FailY",
                          "Texture has dimensions {X}x{Y} and height is not a power of two. Fix the height dimension"),
                Arguments);
            ActionContext->Error(Message);
        }
    }
    else
    {
        LogInfo(Texture, TEXT("Texture dimensions are a power of two. No Aciton required."));
    }
}

void UEnsureTextureResolutionConstraintsAction::CheckDivisibleConstraint(URuleRangerActionContext* ActionContext,
                                                                         const UTexture2D* const Texture) const
{
    const int Divisor = ETextureResolutionConstraint::DivisibleByFour == Constraint ? 4
        : ETextureResolutionConstraint::DivisibleByEight == Constraint              ? 8
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
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "EnsureTextureResolutionConstraintsAction_DivFailXY",
                                                          "Texture has dimensions {X}x{Y} and neither width nor"
                                                          " height is divisible by {Divisor}. Fix both dimensions"),
                                                Arguments);
            ActionContext->Error(Message);
        }
        else if (bInvalidX)
        {
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "EnsureTextureResolutionConstraintsAction_DivFailX",
                                                          "Texture has dimensions {X}x{Y} and width is not "
                                                          "divisible by {Divisor}. Fix the width dimension"),
                                                Arguments);
            ActionContext->Error(Message);
        }
        else
        {
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "EnsureTextureResolutionConstraintsAction_DivFailY",
                                                          "Texture has dimensions {X}x{Y} and height is not "
                                                          "divisible by {Divisor}. Fix the height dimension"),
                                                Arguments);
            ActionContext->Error(Message);
        }
    }
    else
    {
        LogInfo(Texture,
                FString::Printf(TEXT(" Texture dimensions are divisible by %d. No Aciton required."), Divisor));
    }
}

void UEnsureTextureResolutionConstraintsAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                     UObject* Object)
{
    const auto Texture = CastChecked<UTexture2D>(Object);
    if (ETextureResolutionConstraint::PowerOfTwo == Constraint)
    {
        CheckPowerOfTwo(ActionContext, Texture);
    }
    else
    {
        CheckDivisibleConstraint(ActionContext, Texture);
    }
}
