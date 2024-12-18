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
#include "EnsureFunctionsHaveCategoriesAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureFunctionsHaveCategoriesAction)

bool UEnsureFunctionsHaveCategoriesAction::ShouldAnalyzeFunctions(UBlueprint* Blueprint,
                                                                  const TArray<UK2Node_FunctionEntry*>& Functions) const
{
    const int32 FunctionCount = Functions.Num();
    const bool bAnalyze = FunctionCount >= Threshold;
    if (!bAnalyze)
    {
        LogInfo(Blueprint,
                FString::Printf(TEXT("The number of functions in the Blueprint (%d)"
                                     " is below the threshold (%d) so it is not "
                                     "necessary to enforce categorisation of the functions."),
                                FunctionCount,
                                Threshold));
    }
    return bAnalyze;
}

void UEnsureFunctionsHaveCategoriesAction::AnalyzeFunction(URuleRangerActionContext* ActionContext,
                                                           UBlueprint* Blueprint,
                                                           UK2Node_FunctionEntry* FunctionEntry,
                                                           UEdGraph* Graph)
{
    if (FunctionEntry->MetaData.Category.ToString().Equals(TEXT("Default")))
    {
        const auto& ErrorMessage =
            FString::Printf(TEXT("Blueprint contains a function named '%s' that is in the Default "
                                 "category but it is expected to have a non-default category specified "
                                 "as the Blueprint has more than %d functions."),
                            *Graph->GetName(),
                            Threshold);
        ActionContext->Error(FText::FromString(ErrorMessage));
    }
}
