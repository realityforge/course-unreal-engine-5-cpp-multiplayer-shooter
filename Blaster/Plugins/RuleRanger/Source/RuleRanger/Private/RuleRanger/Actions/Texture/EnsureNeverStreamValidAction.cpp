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

#include "EnsureNeverStreamValidAction.h"

void UEnsureNeverStreamValidAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    UTexture2D* Texture = CastChecked<UTexture2D>(Object);
    if (Texture->NeverStream != bNeverStream)
    {
        FFormatNamedArguments Arguments;
        Arguments.Add(TEXT("Original"), FText::FromString(bNeverStream ? TEXT("false") : TEXT("true")));
        Arguments.Add(TEXT("New"), FText::FromString(bNeverStream ? TEXT("true") : TEXT("false")));
        if (ActionContext->IsDryRun())
        {
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "TextureNeverStreamChangeOmitted",
                                                          "Texture should have NeverStream changed from "
                                                          "'{Original}' to '{New}'. "
                                                          "Action skipped in DryRun mode"),
                                                Arguments);
            ActionContext->Warning(Message);
        }
        else
        {
            const FText Message = FText::Format(NSLOCTEXT("RuleRanger",
                                                          "TextureNeverStreamChanged",
                                                          "Texture changed NeverStream from '{Original}' to '{New}'"),
                                                Arguments);
            ActionContext->Info(Message);

            Texture->NeverStream = bNeverStream;

            ensure(Object->MarkPackageDirty());
            ensure(Object->GetOuter()->MarkPackageDirty());
        }
    }
    else
    {
        LogInfo(Texture, TEXT("Texture has a valid NeverStream setting. No Aciton required."));
    }
}
