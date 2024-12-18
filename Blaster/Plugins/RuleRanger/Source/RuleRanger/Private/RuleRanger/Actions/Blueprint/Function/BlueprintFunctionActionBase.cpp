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
#include "BlueprintFunctionActionBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueprintFunctionActionBase)

bool UBlueprintFunctionActionBase::ShouldAnalyzeBlueprint(UBlueprint* Blueprint) const
{
    return true;
}

bool UBlueprintFunctionActionBase::ShouldAnalyzeGraph(UBlueprint* Blueprint, UEdGraph* Graph) const
{
    return UEdGraphSchema_K2::FN_UserConstructionScript != Graph->GetFName();
}

bool UBlueprintFunctionActionBase::ShouldAnalyzeFunction(UBlueprint* Blueprint,
                                                         UK2Node_FunctionEntry* FunctionEntry) const
{
    return true;
}

bool UBlueprintFunctionActionBase::ShouldAnalyzeFunctions(UBlueprint* Blueprint,
                                                          const TArray<UK2Node_FunctionEntry*>& Functions) const
{
    return true;
}

void UBlueprintFunctionActionBase::AnalyzeFunction(URuleRangerActionContext* ActionContext,
                                                   UBlueprint* Blueprint,
                                                   UK2Node_FunctionEntry* FunctionEntry,
                                                   UEdGraph* Graph)
{
    LogError(Blueprint, TEXT("Action failed to override AnalyzeFunction."));
}

void UBlueprintFunctionActionBase::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto Blueprint = CastChecked<UBlueprint>(Object);
    if (BPTYPE_MacroLibrary == Blueprint->BlueprintType)
    {
        LogInfo(Object, TEXT("Object is a MacroLibrary and does not contain any functions."));
    }
    else if (!ShouldAnalyzeBlueprint(Blueprint))
    {
        LogInfo(Object, TEXT("Blueprint not analyzed as ShouldAnalyzeBlueprint() returned false."));
    }
    else
    {
        TArray<UEdGraph*> Graphs;
        TArray<UK2Node_FunctionEntry*> Functions;
        // For all the function graphs that are not construction scripts
        for (const auto Graph : Blueprint->FunctionGraphs)
        {
            if (ShouldAnalyzeGraph(Blueprint, Graph))
            {
                TArray<UK2Node_FunctionEntry*> EntryNodes;
                Graph->GetNodesOfClass(EntryNodes);

                if ((EntryNodes.Num() > 0) && EntryNodes[0]->IsEditable())
                {
                    if (const auto FunctionEntry = EntryNodes[0])
                    {
                        if (ShouldAnalyzeFunction(Blueprint, FunctionEntry))
                        {
                            Graphs.Add(Graph);
                            Functions.Add(FunctionEntry);
                        }
                    }
                }
            }
        }
        if (ShouldAnalyzeFunctions(Blueprint, Functions))
        {
            const int32 Count = Graphs.Num();
            for (int32 i = 0; i < Count; i++)
            {
                const auto Graph = Graphs.Pop();
                const auto Function = Functions.Pop();
                AnalyzeFunction(ActionContext, Blueprint, Function, Graph);
            }
        }
    }
}

UClass* UBlueprintFunctionActionBase::GetExpectedType()
{
    return UBlueprint::StaticClass();
}
