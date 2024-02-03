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

#include "EnsureTextureCompressionValidAction.h"

void UEnsureTextureCompressionValidAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    UTexture2D* Texture = CastChecked<UTexture2D>(Object);
    if (!Settings.Contains(Texture->CompressionSettings))
    {
        const UEnum* Enum = StaticEnum<TextureCompressionSettings>();
        if (bApplyFix && Settings.Num() > 0)
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("OriginalSetting"), Enum->GetDisplayNameTextByValue(Texture->CompressionSettings));
            Arguments.Add(TEXT("NewSetting"), Enum->GetDisplayNameTextByValue(Settings[0]));
            if (ActionContext->IsDryRun())
            {
                const FText Message =
                    FText::Format(NSLOCTEXT("RuleRanger",
                                            "TextureCompressionChangeOmitted",
                                            "Texture should have the compression setting changed from "
                                            "'{OriginalSetting}' to '{NewSetting}'. "
                                            "Action skipped in DryRun mode"),
                                  Arguments);
                ActionContext->Warning(Message);
            }
            else
            {
                const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                              "TextureCompressionChanged",
                                                              "Texture changed the compression setting from "
                                                              "'{OriginalSetting}' to '{NewSetting}'"),
                                                    Arguments);
                ActionContext->Info(Message);

                Texture->CompressionSettings = Settings[0];

                ensure(Object->MarkPackageDirty());
                ensure(Object->GetOuter()->MarkPackageDirty());
            }
        }
        else
        {
            FString ValidSettings{ "" };
            for (const auto& Setting : Settings)
            {
                if (ValidSettings.Len() > 0)
                {
                    ValidSettings.Append(", ");
                }
                ValidSettings.Append(Enum->GetDisplayNameTextByValue(Setting).ToString());
            }
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("ActualSettings"), Enum->GetDisplayNameTextByValue(Texture->CompressionSettings));
            Arguments.Add(TEXT("ValidSettings"), FText::FromString(ValidSettings));
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "EnsureTextureCompressionValidAction_Fail",
                                                          "Texture has the compression '{ActualSettings}' that is not "
                                                          "one of the valid Texture Compressions '{ValidSettings}' and "
                                                          "ApplyFix action parameter has not been specified"),
                                                Arguments);
            ActionContext->Error(Message);
        }
    }
    else
    {
        LogInfo(Texture, TEXT("Texture has a valid Texture Compression Setting. No Aciton required."));
    }
}
