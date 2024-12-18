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

#include "EnsureSRGBValidAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureSRGBValidAction)

void UEnsureSRGBValidAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    UTexture2D* Texture = CastChecked<UTexture2D>(Object);
    if (Texture->SRGB != bSRGB)
    {
        FFormatNamedArguments Arguments;
        Arguments.Add(TEXT("Original"), FText::FromString(bSRGB ? TEXT("false") : TEXT("true")));
        Arguments.Add(TEXT("New"), FText::FromString(bSRGB ? TEXT("true") : TEXT("false")));
        if (ActionContext->IsDryRun())
        {
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "TextureSRGBChangeOmitted",
                                                          "Texture should have sRGB changed from "
                                                          "'{Original}' to '{New}'. "
                                                          "Action skipped in DryRun mode"),
                                                Arguments);
            ActionContext->Warning(Message);
        }
        else
        {
            const FText Message = FText::Format(
                NSLOCTEXT("RuleRanger", "TextureSRGBChanged", "Texture changed sRGB from '{Original}' to '{New}'"),
                Arguments);
            ActionContext->Info(Message);

            Texture->SRGB = bSRGB;

            ensure(Object->MarkPackageDirty());
            ensure(Object->GetOuter()->MarkPackageDirty());
        }
    }
    else
    {
        LogInfo(Texture, TEXT("Texture has a valid sRGB. No Action required."));
    }
}
