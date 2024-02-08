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
#include "RuleRangerActionContext.h"
#include "RuleRangerObjectBase.h"
#include "RuleRangerAction.generated.h"

/**
 * Base class used to apply an action to an object.
 * This is typically used in the body of a rule.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, CollapseCategories, DefaultToInstanced, EditInlineNew)
class RULERANGER_API URuleRangerAction : public URuleRangerObjectBase
{
    GENERATED_BODY()

    friend class URuleRangerRule;

public:
    /**
     * Apply the action to the specified object.
     *
     * @param ActionContext the context in which the action is invoked.
     * @param Object the object to apply the action to.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Rule Ranger")
    void Apply(URuleRangerActionContext* ActionContext, UObject* Object);

    /**
     * Return the type of the object that this action can run on.
     * If the object can not be converted to an object of this type then
     * a error log message is emitted and the action is skipped.
     *
     * @return The Class that the Object must be an instance of.
     */
    virtual UClass* GetExpectedType();
};
