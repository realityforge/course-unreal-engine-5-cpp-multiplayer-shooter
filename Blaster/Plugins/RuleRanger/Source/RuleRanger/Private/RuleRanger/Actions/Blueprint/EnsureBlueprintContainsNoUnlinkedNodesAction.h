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

#pragma once

#include "CoreMinimal.h"
#include "RuleRangerAction.h"
#include "EnsureBlueprintContainsNoUnlinkedNodesAction.generated.h"

/**
 * Action to check that no Blueprint graphs have unlinked nodes.
 * If the node has exec pins then they MUST have input pins filled. If the node has no exec pins then they MUST
 * have output data pins used.
 */
UCLASS(DisplayName = "Ensure Blueprint Contains No Unlinked Nodes")
class RULERANGER_API UEnsureBlueprintContainsNoUnlinkedNodesAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** Should the action generate an error on default Event nodes that have not been deleted nor implemented. */
    UPROPERTY(EditAnywhere)
    bool bErrorOnLooseDefaultEvents{ false };

    bool ShouldHaveLinks(const TObjectPtr<UEdGraphNode> Node) const;

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() override;
};
