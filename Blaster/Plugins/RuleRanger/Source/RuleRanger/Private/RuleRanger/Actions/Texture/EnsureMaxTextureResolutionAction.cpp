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

#include "EnsureMaxTextureResolutionAction.h"

void UEnsureMaxTextureResolutionAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    const auto Texture = CastChecked<UTexture2D>(Object);
    const int32 TexSizeX = Texture->GetSizeX();
    const int32 TexSizeY = Texture->GetSizeY();

    if (TexSizeX > MaxSizeX || TexSizeY > MaxSizeY)
    {
        FFormatNamedArguments Arguments;
        Arguments.Add(TEXT("TexSizeX"), FText::FromString(FString::FromInt(TexSizeX)));
        Arguments.Add(TEXT("TexSizeY"), FText::FromString(FString::FromInt(TexSizeY)));
        Arguments.Add(TEXT("MaxSizeX"), FText::FromString(FString::FromInt(MaxSizeX)));
        Arguments.Add(TEXT("MaxSizeY"), FText::FromString(FString::FromInt(MaxSizeY)));

        ActionContext->Error(FText::Format(
            NSLOCTEXT("RuleRanger",
                      "EnsureMaxTextureResolutionAction_Error",
                      "Texture dimensions {TexSizeX}x{TexSizeY} exceed the maximum dimensions {MaxSizeX}x{MaxSizeY}."),
            Arguments));
    }
}
