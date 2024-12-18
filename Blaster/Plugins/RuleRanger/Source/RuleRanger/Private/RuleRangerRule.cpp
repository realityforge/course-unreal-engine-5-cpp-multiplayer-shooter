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
#include "RuleRanger/RuleRangerUtilities.h"
#include "RuleRangerAction.h"
#include "RuleRangerActionContext.h"
#include "RuleRangerLogging.h"
#include "RuleRangerMatcher.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RuleRangerRule)

void URuleRangerRule::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    if (Match(ActionContext, Object))
    {
        int32 ActionIndex = 0;
        for (const TObjectPtr<URuleRangerAction> Action : Actions)
        {
            if (IsValid(Object))
            {
                if (!IsValid(Action))
                {
                    ActionContext->Error(
                        FText::FromString(FString::Printf(TEXT("Invalid Action detected at index %d in rule '%s'"),
                                                          ActionIndex,
                                                          *GetName())));
                }
                else
                {
                    if (const auto _ = FRuleRangerUtilities::ToObject<UObject>(Object, Action->GetExpectedType()))
                    {
                        Action->Apply(ActionContext, Object);
                    }
                    else
                    {
                        Action->LogError(Object,
                                         FString::Printf(TEXT("Attempt to run on Object that is not an "
                                                              "instance of the type %s."),
                                                         *Action->GetExpectedType()->GetName()));
                    }
                    const auto State = ActionContext->GetState();
                    if (ERuleRangerActionState::AS_Fatal == State)
                    {
                        RR_VERBOSE_ALOG("ApplyRule(%s) on rule %s applied action %s which resulted in fatal error. "
                                        "Processing rules will not continue.",
                                        *Object->GetName(),
                                        *GetName(),
                                        *Action->GetName());
                        return;
                    }
                    else if (!bContinueOnError && ERuleRangerActionState::AS_Error == State)
                    {
                        RR_VERBOSE_ALOG("ApplyRule(%s) on rule %s applied action %s which resulted in error. "
                                        "Processing rules will not continue as ContinueOnError=False.",
                                        *Object->GetName(),
                                        *GetName(),
                                        *Action->GetName());
                        return;
                    }
                }
            }
            ActionIndex++;
        }
    }
}

bool URuleRangerRule::Match(URuleRangerActionContext* ActionContext, UObject* Object) const
{
    int32 MatcherIndex = 0;
    for (const TObjectPtr<URuleRangerMatcher> Matcher : Matchers)
    {
        if (IsValid(Object))
        {
            if (!IsValid(Matcher))
            {
                ActionContext->Error(
                    FText::FromString(FString::Printf(TEXT("Invalid Matcher detected at index %d"), MatcherIndex)));
                return false;
            }
            else
            {
                if (!Matcher->Test(Object))
                {
                    RR_VERBOSE_ALOG("Match(%s) on rule %s exited early as matcher %s did not match",
                                    *Object->GetName(),
                                    *GetName(),
                                    *Matcher->GetName());
                    return false;
                }
            }
        }
        MatcherIndex++;
    }
    return true;
}
