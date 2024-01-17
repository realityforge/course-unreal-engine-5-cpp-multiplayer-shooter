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

#include "EnsureTextureGroupValidAction.h"

void UEnsureTextureGroupValidAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const UTexture2D* Texture = CastChecked<UTexture2D>(Object);
    if (!TextureGroups.Contains(Texture->LODGroup))
    {
        const UEnum* Enum = StaticEnum<TextureGroup>();
        FString ValidGroups{ "" };
        for (const auto& Group : TextureGroups)
        {
            if (ValidGroups.Len() > 0)
            {
                ValidGroups.Append(", ");
            }
            ValidGroups.Append(Enum->GetDisplayNameTextByValue(Group).ToString());
        }
        FFormatNamedArguments Arguments;
        Arguments.Add(TEXT("ActualGroup"), Enum->GetDisplayNameTextByValue(Texture->LODGroup));
        Arguments.Add(TEXT("ValidGroups"), FText::FromString(ValidGroups));
        const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                      "EnsureTextureGroupValidAction_Fail",
                                                      "Texture has the TextureGroup '{ActualGroup}' that is not "
                                                      "one of the valid TextureGroups: '{ValidGroups}'"),
                                            Arguments);
        ActionContext->Error(Message);
    }
    else
    {
        LogInfo(Texture, TEXT("Texture has a valid TextureGroup. No Aciton required."));
    }
}

UClass* UEnsureTextureGroupValidAction::GetExpectedType()
{
    return UTexture2D::StaticClass();
}
