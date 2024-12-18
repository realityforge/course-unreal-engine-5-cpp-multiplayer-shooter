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
#include "K2Node_FunctionEntry.h"
#include "RuleRangerAction.h"
#include "BlueprintFunctionActionBase.generated.h"

/**
 * Base class for analyzing the functions in a Blueprint Graph.
 */
UCLASS(Abstract)
class RULERANGER_API UBlueprintFunctionActionBase : public URuleRangerAction
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
     * @param Blueprint the Blueprint.
     * @param Graph the function Graph.
     * @return true to analyze function, false otherwise.
     */
    virtual bool ShouldAnalyzeGraph(UBlueprint* Blueprint, UEdGraph* Graph) const;

    /**
     * Return true to analyze the particular Blueprint Function.
     * Useful to filter functions from analysis.
     *
     * @param Blueprint the Blueprint.
     * @param FunctionEntry The node representing the function entry point.
     * @return true to analyze function, false otherwise.
     */
    virtual bool ShouldAnalyzeFunction(UBlueprint* Blueprint, UK2Node_FunctionEntry* FunctionEntry) const;

    /**
     * Return true to analyze the Functions.
     * This has already been filtered down by ShouldAnalyzeFunction, and thus all functions will be analyzed if this
     * function returns true.
     *
     * @param Blueprint the Blueprint.
     * @param Functions The nodes representing the function entry points.
     * @return true to analyze functions, false otherwise.
     */
    virtual bool ShouldAnalyzeFunctions(UBlueprint* Blueprint, const TArray<UK2Node_FunctionEntry*>& Functions) const;

    /**
     * Function to override to analyze a particular function.
     *
     * @param ActionContext The ActionContext.
     * @param Blueprint The root object (a.k.a. the Blueprint)
     * @param FunctionEntry The node representing the function entry point.
     * @param Graph The Function graph.
     */
    virtual void AnalyzeFunction(URuleRangerActionContext* ActionContext,
                                 UBlueprint* Blueprint,
                                 UK2Node_FunctionEntry* FunctionEntry,
                                 UEdGraph* Graph);

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() override;
};
