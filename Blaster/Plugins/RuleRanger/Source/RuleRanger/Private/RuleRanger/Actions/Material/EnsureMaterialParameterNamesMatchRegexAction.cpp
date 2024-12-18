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
#include "EnsureMaterialParameterNamesMatchRegexAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureMaterialParameterNamesMatchRegexAction)

void UEnsureMaterialParameterNamesMatchRegexAction::AnalyzeParameter(URuleRangerActionContext* ActionContext,
                                                                     const UMaterial* Material,
                                                                     const FMaterialParameterInfo& Info,
                                                                     const FMaterialParameterMetadata& Metadata) const
{
    const FString& Name = Info.Name.ToString();
    const FRegexPattern RegexPattern(Pattern,
                                     bCaseSensitive ? ERegexPatternFlags::None : ERegexPatternFlags::CaseInsensitive);
    // ReSharper disable once CppTooWideScopeInitStatement
    FRegexMatcher Matcher(RegexPattern, Name);
    if (!Matcher.FindNext())
    {
        ActionContext->Error(FText::FromString(
            static_cast<const FString&>(FString::Printf(TEXT("Material contains a parameter named "
                                                             "'%s' that does not match the regex %s."),
                                                        *Name,
                                                        *Pattern))));
    }
    else
    {
        LogInfo(Material, FString::Printf(TEXT("Material Parameter named %s matches pattern %s"), *Name, *Pattern));
    }
}
