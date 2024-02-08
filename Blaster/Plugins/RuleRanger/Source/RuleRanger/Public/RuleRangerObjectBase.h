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
#include "RuleRangerObjectBase.generated.h"

/**
 * Base class used for actions and matchers.
 */
UCLASS(Abstract, AutoExpandCategories = ("Rule Ranger"))
class RULERANGER_API URuleRangerObjectBase : public UObject
{
    GENERATED_BODY()

protected:
    /**
     * Log an informational message for debugging purposes.
     * (By default this logs to the RuleRanger category using VeryVerbose level.)
     *
     * @param Object the Object that was being processed (if any).
     * @param Message the message.
     */
    void LogInfo(const UObject* const Object, const FString& Message) const;

    /**
     * Log an error message for debugging purposes.
     * This usually indicates a misconfigured object.
     * (By default this logs to the RuleRanger category using Error level.)
     *
     * @param Object the Object that was being processed (if any).
     * @param Message the message.
     */
    void LogError(const UObject* const Object, const FString& Message) const;
};
