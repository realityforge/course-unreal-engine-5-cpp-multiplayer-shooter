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
#include "BaseAnalyzeVariableAction.generated.h"

/**
 * Base class for analyzing the variables in a Blueprint.
 */
UCLASS(Abstract, AutoExpandCategories = ("Rule Ranger"))
class RULERANGER_API UBaseAnalyzeVariableAction : public URuleRangerAction
{
    GENERATED_BODY()

protected:
    /**
     * Return true to ensure this particular Blueprint is analyzed.
     * Useful to return false if a threshold of complexity needs to be reached before the action is applied.
     *
     * @param Blueprint the Blueprint.
     * @return true to analyze Blueprint, otherwise false.
     */
    virtual bool ShouldAnalyzeBlueprint(UBlueprint* Blueprint) const;

    /**
     * Return true to analyze the particular Blueprint Graph/Function.
     * Useful to skip Construction graphs that are automatically created and can not be modified.
     *
     * @param Graph the function Graph.
     * @return true to analyze function, false otherwise.
     */
    virtual bool ShouldAnalyzeGraph(UEdGraph* Graph) const;

    /**
     * Function to override to analyze a particular variable.
     *
     * @param ActionContext The ActionContext.
     * @param Blueprint The root object (a.k.a. the Blueprint)
     * @param Variable The Variable.
     * @param Graph The Function graph if a local variable else nullptr.
     */
    virtual void AnalyzeVariable(URuleRangerActionContext* ActionContext,
                                 UBlueprint* Blueprint,
                                 const FBPVariableDescription& Variable,
                                 UEdGraph* Graph);

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() override;
};
