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
#include "CheckBlueprintCompileStatusAction.h"
#include "BlueprintCompilationManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CheckBlueprintCompileStatusAction)

void UCheckBlueprintCompileStatusAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto Blueprint = CastChecked<UBlueprint>(Object);
    if ((BS_Dirty == Blueprint->Status || BS_Unknown == Blueprint->Status) && !ActionContext->IsDryRun())
    {
        LogInfo(Object,
                FString::Printf(TEXT("Blueprint compile status is %s, attempting to recompile "
                                     "as RuleRanger is not in DryRun mode."),
                                (BS_Dirty == Blueprint->Status ? TEXT("Dirty") : TEXT("Unknown"))));
        if (!Blueprint->bBeingCompiled)
        {
            const FBPCompileRequest Request(Blueprint, EBlueprintCompileOptions::None, nullptr);
            FBlueprintCompilationManager::CompileSynchronously(Request);
        }
        else
        {
            LogInfo(Object,
                    FString::Printf(TEXT("Blueprint is being compiled. "
                                         "Waiting for compilation to complete...")));
            FBlueprintCompilationManager::FlushCompilationQueue(nullptr);
        }
        LogInfo(Object, FString::Printf(TEXT("Blueprint  compilation complete.")));
    }

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

UClass* UCheckBlueprintCompileStatusAction::GetExpectedType()
{
    return UBlueprint::StaticClass();
}
