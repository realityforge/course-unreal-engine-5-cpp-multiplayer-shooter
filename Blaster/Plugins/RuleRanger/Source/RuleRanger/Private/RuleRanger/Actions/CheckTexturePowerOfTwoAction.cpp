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

#include "CheckTexturePowerOfTwoAction.h"
#include "Editor.h"

UCheckTexturePowerOfTwoAction::UCheckTexturePowerOfTwoAction()
{
    TextureGroupsToSkip.Add(TEXTUREGROUP_UI);
}

void UCheckTexturePowerOfTwoAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    if (IsValid(Object))
    {
        if (const auto Texture = Cast<UTexture2D>(Object); !Texture)
        {
            LogError(Object, TEXT("Attempt to run on Object that is not a Texture2D instance."));
        }
        else if (TextureGroupsToSkip.Contains(Texture->LODGroup))
        {
            const auto TextureGroupEnum = StaticEnum<TextureGroup>();
            const auto TextureGroup = TextureGroupEnum->GetMetaData(TEXT("DisplayName"), Texture->LODGroup);
            LogInfo(
                Object,
                FString::Printf(TEXT("Skipping object as it's TextureGroup %s is in the skip list"), *TextureGroup));
        }
        else
        {
            const int32 SizeX = Texture->GetSizeX();
            const int32 SizeY = Texture->GetSizeY();

            const bool bInvalidX = (0 != (SizeX & SizeX - 1));
            const bool bInvalidY = (0 != (SizeY & SizeY - 1));

            if (bInvalidX || bInvalidY)
            {
                const auto TextureGroupEnum = StaticEnum<TextureGroup>();
                FString TextureGroupsLabel;
                for (TEnumAsByte<TextureGroup> TextureGroup : TextureGroupsToSkip)
                {
                    if (!TextureGroupsLabel.IsEmpty())
                    {
                        TextureGroupsLabel.Append(TEXT(", "));
                    }
                    TextureGroupsLabel.Append(TextureGroupEnum->GetMetaData(TEXT("DisplayName"), TextureGroup));
                }
                FFormatNamedArguments AltActionArguments;
                AltActionArguments.Add(TEXT("Groups"), FText::FromString(TextureGroupsLabel));
                const FText AltAction = TextureGroupsLabel.IsEmpty()
                    ? FText::GetEmpty()
                    : FText::Format(NSLOCTEXT("RuleRanger",
                                              "CheckTexture2DPowerOfTwoAction_AltAction",
                                              " or assign the texture to one of the LOD groups: {Groups}"),
                                    AltActionArguments);

                FFormatNamedArguments Arguments;
                Arguments.Add(TEXT("X"), FText::FromString(FString::FromInt(SizeX)));
                Arguments.Add(TEXT("Y"), FText::FromString(FString::FromInt(SizeY)));
                Arguments.Add(TEXT("AltAction"), AltAction);

                if (bInvalidX && bInvalidY)
                {
                    const FText Message =
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "CheckTexture2DPowerOfTwoAction_FailXY",
                                                "Texture has dimensions {X}x{Y} and neither width nor"
                                                " height is a power of two. Fix both dimensions{AltAction}"),
                                      Arguments);
                    ActionContext->Error(Message);
                }
                else if (bInvalidX)
                {
                    const FText Message = FText::Format(
                        NSLOCTEXT(
                            "RuleRanger",
                            "CheckTexture2DPowerOfTwoAction_FailX",
                            "Texture has dimensions {X}x{Y} and width is not a power of two. Fix the width dimension{AltAction}"),
                        Arguments);
                    ActionContext->Error(Message);
                }
                else
                {
                    const FText Message = FText::Format(
                        NSLOCTEXT(
                            "RuleRanger",
                            "CheckTexture2DPowerOfTwoAction_FailX",
                            "Texture has dimensions {X}x{Y} and height is not a power of two. Fix the height dimension{AltAction}"),
                        Arguments);
                    ActionContext->Error(Message);
                }
            }
            else
            {
                LogInfo(Object, TEXT(" Texture dimensions are a power of two. No Aciton required."));
            }
        }
    }
}
