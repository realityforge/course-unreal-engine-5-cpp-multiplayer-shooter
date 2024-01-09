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

#include "EnsureFunctionsHaveDescriptionsAction.h"
#include "K2Node_FunctionEntry.h"

void UEnsureFunctionsHaveDescriptionsAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                  UObject* Object)
{
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
            if (UEdGraphSchema_K2::FN_UserConstructionScript != Graph->GetFName())
            {
                TArray<UK2Node_FunctionEntry*> EntryNodes;
                Graph->GetNodesOfClass(EntryNodes);

                if ((EntryNodes.Num() > 0) && EntryNodes[0]->IsEditable())
                {
                    if (const auto FunctionEntry = EntryNodes[0])
                    {
                        if (FunctionEntry->MetaData.ToolTip.IsEmpty())
                        {
                            const bool bPublic = FUNC_Public & FunctionEntry->GetFunctionFlags();
                            // ReSharper disable once CppTooWideScopeInitStatement
                            const bool bProtected = FUNC_Protected & FunctionEntry->GetFunctionFlags();
                            if (bPublic || (bCheckProtectedFunctions && bProtected))
                            {
                                const auto& ErrorMessage = FString::Printf(
                                    TEXT(
                                        "Blueprint contains a %s function named '%s' that is expected to have a description."),
                                    (bPublic ? TEXT("public") : TEXT("protected")),
                                    *FunctionEntry->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
                                ActionContext->Error(FText::FromString(ErrorMessage));
                            }
                        }
                    }
                }
            }
        }
    }
}

UClass* UEnsureFunctionsHaveDescriptionsAction::GetExpectedType()
{
    return UBlueprint::StaticClass();
}
