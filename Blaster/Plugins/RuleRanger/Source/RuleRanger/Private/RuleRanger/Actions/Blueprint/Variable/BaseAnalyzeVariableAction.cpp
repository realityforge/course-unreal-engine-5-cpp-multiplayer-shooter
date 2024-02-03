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

#include "BlueprintVariableActionBase.h"
#include "K2Node_FunctionEntry.h"

bool UBlueprintVariableActionBase::ShouldAnalyzeBlueprint(UBlueprint* Blueprint) const
{
    return true;
}

bool UBlueprintVariableActionBase::ShouldAnalyzeBlueprintVariables(UBlueprint* Blueprint) const
{
    return true;
}

bool UBlueprintVariableActionBase::ShouldAnalyzeFunction(UEdGraph* Graph, UK2Node_FunctionEntry* FunctionEntry) const
{
    return UEdGraphSchema_K2::FN_UserConstructionScript != Graph->GetFName();
}

void UBlueprintVariableActionBase::AnalyzeVariable(URuleRangerActionContext* ActionContext,
                                                   UBlueprint* Blueprint,
                                                   const FBPVariableDescription& Variable,
                                                   UK2Node_FunctionEntry* FunctionEntry,
                                                   UEdGraph* Graph)
{
    LogError(Blueprint, TEXT("Action failed to override AnalyzeVariable."));
}

void UBlueprintVariableActionBase::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto Blueprint = CastChecked<UBlueprint>(Object);
    if (BPTYPE_MacroLibrary == Blueprint->BlueprintType)
    {
        LogInfo(Object, TEXT("Object is a MacroLibrary and does not contain any functions or variables."));
    }
    else if (!ShouldAnalyzeBlueprint(Blueprint))
    {
        LogInfo(Object, TEXT("Blueprint not analyzed as ShouldAnalyzeBlueprint() returned false."));
    }
    else
    {
        if (ShouldAnalyzeBlueprintVariables(Blueprint))
        {
            for (auto& Variable : Blueprint->NewVariables)
            {
                AnalyzeVariable(ActionContext, Blueprint, Variable, nullptr, nullptr);
            }
        }
        // For all the function graphs that are not construction scripts
        for (const auto Graph : Blueprint->FunctionGraphs)
        {
            TArray<UK2Node_FunctionEntry*> EntryNodes;
            Graph->GetNodesOfClass(EntryNodes);

            if ((EntryNodes.Num() > 0) && EntryNodes[0]->IsEditable() && ShouldAnalyzeFunction(Graph, EntryNodes[0]))
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                const auto FunctionEntry = EntryNodes[0];
                for (auto& Variable : FunctionEntry->LocalVariables)
                {
                    AnalyzeVariable(ActionContext, Blueprint, Variable, FunctionEntry, Graph);
                }
            }
        }
    }
}

UClass* UBlueprintVariableActionBase::GetExpectedType()
{
    return UBlueprint::StaticClass();
}
