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

#include "EnsureMatchersMatchAction.h"

void UEnsureMatchersMatchAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    int Index = 0;
    for (const auto& Matcher : Matchers)
    {
        Index++;
        if (IsValid(Matcher))
        {
            if (!Matcher->Test(Object))
            {
                if (Message.IsEmpty())
                {
                    FFormatNamedArguments Arguments;
                    Arguments.Add(TEXT("MatcherName"), FText::FromString(*Matcher->GetName()));
                    Arguments.Add(TEXT("Index"), FText::FromString(FString::FromInt(Index)));

                    ActionContext->Error(FText::Format(NSLOCTEXT("RuleRanger",
                                                                 "EnsureMatchersAction_FailedMatcher",
                                                                 "Failed matcher {MatcherName} at index {Index}."),
                                                       Arguments));
                }
                else
                {

                    ActionContext->Error(FText::FromString(Message));
                }
            }
        }
        else
        {
            LogInfo(Object, FString::Printf(TEXT("Invaliid matcher detected at index %d"), Index));
        }
    }
}
