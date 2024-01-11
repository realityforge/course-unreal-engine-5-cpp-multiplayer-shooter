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

#include "BaseAnalyzeFunctionAction.h"
bool UBaseAnalyzeFunctionAction::ShouldAnalyzeGraph(const TObjectPtr<UEdGraph> Graph) const
{
    return UEdGraphSchema_K2::FN_UserConstructionScript != Graph->GetFName();
}

void UBaseAnalyzeFunctionAction::AnalyzeFunction(URuleRangerActionContext* ActionContext,
                                                 UObject* Object,
                                                 UK2Node_FunctionEntry* FunctionEntry,
                                                 UEdGraph* Graph)
{
    LogError(Object, TEXT("Action failed to override AnalyzeFunction."));
}

void UBaseAnalyzeFunctionAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto Blueprint = CastChecked<UBlueprint>(Object);
    if (BPTYPE_MacroLibrary == Blueprint->BlueprintType)
    {
        LogInfo(Object, TEXT("Object is a MacroLibrary and does not contain any functions."));
    }
    else
    {
        // For all the function graphs that are not construction scripts
        for (const auto Graph : Blueprint->FunctionGraphs)
        {
            if (ShouldAnalyzeGraph(Graph))
            {
                TArray<UK2Node_FunctionEntry*> EntryNodes;
                Graph->GetNodesOfClass(EntryNodes);

                if ((EntryNodes.Num() > 0) && EntryNodes[0]->IsEditable())
                {
                    if (const auto FunctionEntry = EntryNodes[0])
                    {
                        AnalyzeFunction(ActionContext, Object, FunctionEntry, Graph);
                    }
                }
            }
        }
    }
}

UClass* UBaseAnalyzeFunctionAction::GetExpectedType()
{
    return UBlueprint::StaticClass();
}
