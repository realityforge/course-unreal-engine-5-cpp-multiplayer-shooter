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

#include "EnsureVariableNamesMatchRegexAction.h"

void UEnsureVariableNamesMatchRegexAction::AnalyzeVariable(URuleRangerActionContext* ActionContext,
                                                           UBlueprint* Blueprint,
                                                           const FBPVariableDescription& Variable,
                                                           UK2Node_FunctionEntry* FunctionEntry,
                                                           UEdGraph* Graph)
{
    const FString& Name = Variable.VarName.ToString();
    const FRegexPattern RegexPattern(Pattern,
                                     bCaseSensitive ? ERegexPatternFlags::None : ERegexPatternFlags::CaseInsensitive);
    // ReSharper disable once CppTooWideScopeInitStatement
    FRegexMatcher Matcher(RegexPattern, Name);
    if (!Matcher.FindNext())
    {
        if (Graph)
        {
            const auto& ErrorMessage = FString::Printf(TEXT("Blueprint contains a local variable named "
                                                            "'%s' in the function named '%s' "
                                                            "that does not match the regex %s."),
                                                       *Name,
                                                       *Graph->GetName(),
                                                       *Pattern);
            ActionContext->Error(FText::FromString(ErrorMessage));
        }
        else
        {
            const auto& ErrorMessage =
                FString::Printf(TEXT("Blueprint contains a variable named '%s' that does not match the regex %s."),
                                *Name,
                                *Pattern);
            ActionContext->Error(FText::FromString(ErrorMessage));
        }
    }
    else
    {
        LogInfo(Blueprint, FString::Printf(TEXT("Variable named %s matches pattern %s"), *Name, *Pattern));
    }
}
