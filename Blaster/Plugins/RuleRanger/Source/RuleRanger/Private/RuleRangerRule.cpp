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
#include "RuleRangerRule.h"
#include "RuleRangerAction.h"
#include "RuleRangerActionContext.h"
#include "RuleRangerLogging.h"
#include "RuleRangerMatcher.h"

void URuleRangerRule::Apply_Implementation(TScriptInterface<IRuleRangerActionContext>& ActionContext, UObject* Object)
{
    for (const TObjectPtr<URuleRangerMatcher> Matcher : Matchers)
    {
        if (!Matcher->Test(Object))
        {
            UE_LOG(RuleRanger,
                   Verbose,
                   TEXT("ApplyRule(%s) on rule %s exited early as matcher %s did not match"),
                   *Object->GetName(),
                   *GetName(),
                   *Matcher->GetName());
            return;
        }
    }

    for (const TObjectPtr<URuleRangerAction> Action : Actions)
    {
        Action->Apply(ActionContext, Object);
        const auto State = ActionContext->GetState();
        if (ERuleRangerActionState::AS_Fatal == State)
        {
            UE_LOG(RuleRanger,
                   Verbose,
                   TEXT("ApplyRule(%s) on rule %s applied action %s which resulted in fatal error. "
                        "Processing rules will not continue."),
                   *Object->GetName(),
                   *GetName(),
                   *Action->GetName());
            return;
        }
        else if (!bContinueOnError && ERuleRangerActionState::AS_Error == State)
        {
            UE_LOG(RuleRanger,
                   Verbose,
                   TEXT("ApplyRule(%s) on rule %s applied action %s which resulted in error. "
                        "Processing rules will not continue as ContinueOnError=False."),
                   *Object->GetName(),
                   *GetName(),
                   *Action->GetName());
            return;
        }
    }
}
