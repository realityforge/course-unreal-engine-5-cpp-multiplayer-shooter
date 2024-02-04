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

#include "EnsureFunctionsReturnAction.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"

bool UEnsureFunctionsReturnAction::ShouldAnalyzeGraph(UEdGraph* Graph) const
{
    return Super::ShouldAnalyzeGraph(Graph) && UEdGraphSchema_K2::GN_AnimGraph != Graph->GetFName();
}

void UEnsureFunctionsReturnAction::AnalyzeFunction(URuleRangerActionContext* ActionContext,
                                                       UBlueprint* Blueprint,
                                                       UK2Node_FunctionEntry* FunctionEntry,
                                                       UEdGraph* Graph)
{
    // Find the list of result nodes
    TArray<UK2Node_FunctionResult*> ResultNodes;
    Graph->GetNodesOfClass(ResultNodes);
    if (ResultNodes.Num() <= 0)
    {
        const auto& ErrorMessage =
            FString::Printf(TEXT("Blueprint contains a function named '%s' that is missing a return node."),
                            *Graph->GetName());
        ActionContext->Error(FText::FromString(ErrorMessage));
    }
}
