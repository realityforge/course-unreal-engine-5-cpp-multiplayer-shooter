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
#include "EnsureBlueprintContainsNoUnlinkedNodesAction.h"
#include "AnimGraphNode_TransitionResult.h"
#include "AnimStateTransitionNode.h"
#include "EdGraphNode_Comment.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureBlueprintContainsNoUnlinkedNodesAction)

bool UEnsureBlueprintContainsNoUnlinkedNodesAction::ShouldHaveLinks(const TObjectPtr<UEdGraphNode> Node) const
{
    if (!bErrorOnLooseDefaultEvents && Node->IsAutomaticallyPlacedGhostNode())
    {
        // Default Event nodes should not cause issues if they are a default ane we have set
        // the flag and they have no incoming links
        return false;
    }
    else if (Node->IsA(UEdGraphNode_Comment::StaticClass()))
    {
        // Comments have no  incoming links
        return false;
    }
    else if (Node->GetGraph()->GetFName() == UEdGraphSchema_K2::FN_UserConstructionScript
             && Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString().Equals("Construction Script"))
    {
        // Perfectly happy with empty construction script as can not delete it
        return false;
    }
    else if (const auto AnimStateTransitionNode = Cast<UAnimGraphNode_TransitionResult>(Node))
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto Outer = CastChecked<UAnimStateTransitionNode>(AnimStateTransitionNode->GetGraph()->GetOuter());
        // If the transition is automatic then it does not matter that the result node is not connected as it
        // will never be checked but you can not delete it
        return !Outer->bAutomaticRuleBasedOnSequencePlayerInState;
    }
    else
    {
        // Analyze this node
        return true;
    }
}

void UEnsureBlueprintContainsNoUnlinkedNodesAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                         UObject* Object)
{
    const auto Blueprint = CastChecked<UBlueprint>(Object);
    TArray<UEdGraph*> Graphs;
    Blueprint->GetAllGraphs(Graphs);

    for (const auto Graph : Graphs)
    {
        for (const auto Node : Graph->Nodes)
        {
            if (ShouldHaveLinks(Node))
            {
                // If the node has no exec pins then it is pure and must have a
                // consumer of one (Data) output pin. If a node is not pure then
                // it must be connected by one input exec pin
                bool bHasOutputExecPin = false;
                bool bHasInputExecPin = false;
                bool bHasInputPins = false;
                bool bHasOutputPins = false;
                bool bHasInputExecNotLinked = false;
                bool bHasOutputExecNotLinked = false;
                int32 InputExecLinkCount = 0;
                for (const auto Pin : Node->GetAllPins())
                {
                    // Exec pins in controlRig are identified
                    const bool bIsExecPin =
                        UEdGraphSchema_K2::IsExecPin(*Pin) || Pin->GetName().EndsWith(TEXT(".ExecuteContext"));
                    const bool bIsInputPin = EGPD_Input == Pin->Direction;
                    const bool bIsOutputPin = EGPD_Output == Pin->Direction;
                    const bool bPinLinked = 0 != Pin->LinkedTo.Num();
                    bHasInputPins |= bIsInputPin;
                    bHasOutputPins |= bIsOutputPin;
                    bHasInputExecPin |= bIsInputPin && bIsExecPin;
                    bHasOutputExecPin |= bIsOutputPin && bIsExecPin;
                    if (bIsInputPin && bIsExecPin)
                    {
                        if (bPinLinked)
                        {
                            // Some nodes have multiple Input Exec pins (i.e. Timeline node).
                            // As long as at least one is connected then we consider the node used
                            // so we also track the count that are linked and use it later
                            InputExecLinkCount++;
                        }
                        else
                        {
                            bHasInputExecNotLinked = true;
                        }
                    }
                    else if (bIsOutputPin && bIsExecPin)
                    {
                        if (!bPinLinked)
                        {
                            bHasOutputExecNotLinked = true;
                        }
                    }
                }

                if (bHasInputExecNotLinked || bHasOutputExecNotLinked)
                {
                    const FString NodeTitle = Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString();
                    FGraphDisplayInfo Info;
                    Graph->GetSchema()->GetGraphDisplayInformation(*Graph, Info);
                    const FString GraphName = Info.DisplayName.ToString();

                    if (Node->IsAutomaticallyPlacedGhostNode())
                    {
                        ActionContext->Error(FText::FromString(FString::Printf(
                            TEXT("Blueprint has a default event node named "
                                 "'%s' in '%s' with no outgoing links and bErrorOnLooseDefaultEvents=true. "
                                 "Remove event node or change bErrorOnLooseDefaultEvents to false."),
                            *NodeTitle,
                            *GraphName)));
                    }
                    else if (bHasInputExecNotLinked && 0 == InputExecLinkCount)
                    {
                        ActionContext->Error(FText::FromString(
                            FString::Printf(TEXT("Blueprint has a node named '%s' in '%s' missing an exec input pin. "
                                                 "Remove node or connect pin."),
                                            *NodeTitle,
                                            *GraphName)));
                    }
                }
            }
        }
    }

    if (ERuleRangerActionState::AS_Success == ActionContext->GetState())
    {
        LogInfo(Object, TEXT("Blueprint does not have any loose nodes. Blueprint passes check."));
    }
}

UClass* UEnsureBlueprintContainsNoUnlinkedNodesAction::GetExpectedType()
{
    return UBlueprint::StaticClass();
}
