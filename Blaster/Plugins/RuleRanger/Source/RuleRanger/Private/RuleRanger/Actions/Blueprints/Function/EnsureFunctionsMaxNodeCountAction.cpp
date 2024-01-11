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

#include "EnsureFunctionsMaxNodeCountAction.h"
#include "EdGraphNode_Comment.h"
#include "K2Node_BreakStruct.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CastByteToEnum.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_Knot.h"
#include "K2Node_Self.h"
#include "K2Node_StructMemberGet.h"
#include "K2Node_TemporaryVariable.h"
#include "K2Node_Tunnel.h"
#include "K2Node_VariableGet.h"

bool UEnsureFunctionsMaxNodeCountAction::IsNodeTrivial(const UEdGraphNode* Node)
{
    // The algorithm for determining which nodes are "trivial" was initially extracted from the Linter
    // plugin from the lint rule "ULintRule_Blueprint_Funcs_MaxNodes". It will likely diverge over time.
    if (Node->IsA(UK2Node_Knot::StaticClass()) || Node->IsA(UK2Node_FunctionEntry::StaticClass())
        || Node->IsA(UK2Node_Self::StaticClass()) || Node->IsA(UK2Node_DynamicCast::StaticClass())
        || Node->IsA(UK2Node_BreakStruct::StaticClass()) || Node->IsA(UEdGraphNode_Comment::StaticClass())
        || Node->IsA(UK2Node_VariableGet::StaticClass()) || Node->IsA(UK2Node_StructMemberGet::StaticClass())
        || Node->IsA(UK2Node_Tunnel::StaticClass()) || Node->IsA(UK2Node_TemporaryVariable::StaticClass())
        || Node->IsA(UK2Node_FunctionResult::StaticClass()) || Node->IsA(UK2Node_FunctionTerminator::StaticClass())
        || Node->IsA(UK2Node_CastByteToEnum::StaticClass()))
    {
        return true;
    }
    else if (const UK2Node_CallFunction* CallFuncNode = Cast<UK2Node_CallFunction>(Node))
    {
        const FName& FuncName = CallFuncNode->FunctionReference.GetMemberName();
        return FuncName == TEXT("Conv_InterfaceToObject") || FuncName.ToString().Contains(TEXT("MakeLiteral"))
            || FuncName.ToString().Contains(TEXT("ToString")) || FuncName.ToString().StartsWith(TEXT("Make"))
            || FuncName.ToString().StartsWith(TEXT("Break"));
    }
    else
    {
        return false;
    }
}

void UEnsureFunctionsMaxNodeCountAction::AnalyzeFunction(URuleRangerActionContext* ActionContext,
                                                         UBlueprint* Blueprint,
                                                         UK2Node_FunctionEntry* FunctionEntry,
                                                         UEdGraph* Graph)
{
    // Check whether the number of nodes in graph is below limit and if not then count
    // non-trivial nodes and check whether it is below the limit
    if (Graph->Nodes.Num() > MaxNonTrivialNodeCount)
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        const int32 NonTrivialNodeCount =
            Graph->Nodes.FilterByPredicate([this](const UEdGraphNode* Node) { return !IsNodeTrivial(Node); }).Num();
        if (NonTrivialNodeCount > MaxNonTrivialNodeCount)
        {
            const auto& ErrorMessage =
                FString::Printf(TEXT("Blueprint contains a function named '%s' that contains %d non-trivial "
                                     "nodes which exceeds the limit of %d non-trivial nodes."),
                                *Graph->GetName(),
                                NonTrivialNodeCount,
                                MaxNonTrivialNodeCount);
            ActionContext->Error(FText::FromString(ErrorMessage));
        }
    }
}
