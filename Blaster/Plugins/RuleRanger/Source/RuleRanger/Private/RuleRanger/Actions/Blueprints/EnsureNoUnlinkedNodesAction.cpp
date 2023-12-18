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

#include "EnsureNoUnlinkedNodesAction.h"
#include "AnimGraphNode_TransitionResult.h"
#include "AnimStateNode.h"
#include "AnimStateTransitionNode.h"
#include "EdGraphNode_Comment.h"

bool UEnsureNoUnlinkedNodesAction::ShouldHaveLinks(const TObjectPtr<UEdGraphNode> Node) const
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

void UEnsureNoUnlinkedNodesAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    if (IsValid(Object))
    {
        if (const auto Blueprint = Cast<UBlueprint>(Object); !Blueprint)
        {
            LogError(Object, TEXT("Attempt to run on Object that is not a Blueprint instance."));
        }
        else
        {
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
                        bool bHasExecPin = false;
                        bool bInputPin = false;
                        bool bOutputPin = false;
                        for (const auto Pin : Node->GetAllPins())
                        {
                            if (UEdGraphSchema_K2::IsExecPin(*Pin))
                            {
                                bHasExecPin = true;
                            }
                            bInputPin |= Pin->Direction == EGPD_Input;
                            bOutputPin |= Pin->Direction == EGPD_Output;
                        }

                        bool bInputExecNotLinked = false;
                        bool bOutputNonExecLinked = false;
                        for (const auto Pin : Node->GetAllPins())
                        {
                            const bool bPinLinked = 0 != Pin->LinkedTo.Num();
                            if (EGPD_Output == Pin->Direction && bPinLinked)
                            {
                                bOutputNonExecLinked = true;
                            }
                            else if (EGPD_Input == Pin->Direction
                                     && (UEdGraphSchema_K2::IsExecPin(*Pin) && !bPinLinked))
                            {
                                bInputExecNotLinked = true;
                            }
                        }

                        if (Node.IsA(UAnimStateNode::StaticClass()))
                        {
                            // State Machine Nodes always have outputs and we do not care
                            // when they are blank so we fake a link to avoid the error
                            bOutputNonExecLinked = true;
                        }

                        if (bInputExecNotLinked || (!bOutputNonExecLinked && !bHasExecPin && bOutputPin))
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
                            else if (bInputExecNotLinked)
                            {
                                ActionContext->Error(FText::FromString(FString::Printf(
                                    TEXT("Blueprint has a node named '%s' in '%s' missing an exec input pin. "
                                         "Remove node or connect pin."),
                                    *NodeTitle,
                                    *GraphName)));
                            }
                            else // (!bOutputNonExecLinked && !bHasExecPin && bOutputPin)
                            {
                                ActionContext->Error(FText::FromString(FString::Printf(
                                    TEXT(
                                        "Blueprint has a node named '%s' in '%s' that has no output pin linked but has no exec pin. "
                                        "Remove node or add a link to output pin."),
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
    }
}
