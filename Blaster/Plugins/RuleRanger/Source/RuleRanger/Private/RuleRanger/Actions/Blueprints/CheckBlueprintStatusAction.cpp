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

#include "CheckBlueprintStatusAction.h"

void UCheckBlueprintStatusAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    const auto Blueprint = CastChecked<UBlueprint>(Object);
    switch (Blueprint->Status)
    {
        case BS_BeingCreated:
            LogInfo(Object, TEXT("Blueprint status is BeingCreated. Status valid."));
            return;

        case BS_Dirty:
            ActionContext->Error(FText::FromString("Blueprint is dirty and needs to be recompiled"));
            return;

        case BS_UpToDate:
            LogInfo(Object, TEXT("Blueprint status is UpToDate. Status valid."));
            return;
        case BS_Error:
            ActionContext->Error(
                FText::FromString("Blueprint has an Unknown status. Fix error and recompile Blueprint."));
            return;
        case BS_UpToDateWithWarnings:
            if (bErrorOnUpToDateWithWarnings)
            {
                ActionContext->Error(FText::FromString(
                    "Blueprint is UpToDate but has warnings. Fix warnings and recompile or set bErrorOnUpToDateWithWarnings=false"));
            }
            else
            {
                LogInfo(Object,
                        TEXT("Blueprint status is Unknown and bErrorOnUpToDateWithWarnings=false. Status valid."));
            }
            return;
        case BS_Unknown:
        default:
            if (bErrorOnUnknown)
            {
                ActionContext->Error(FText::FromString("Blueprint has an Unknown status. Recompile Blueprint"));
            }
            else
            {
                LogInfo(Object, TEXT("Blueprint status is Unknown and bErrorOnUnknown=false. Status valid."));
            }
    }
}

UClass* UCheckBlueprintStatusAction::GetExpectedType()
{
    return UBlueprint::StaticClass();
}
