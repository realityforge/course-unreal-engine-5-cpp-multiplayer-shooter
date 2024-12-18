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
#include "BlueprintFunctionAccessFilteringActionBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueprintFunctionAccessFilteringActionBase)

bool UBlueprintFunctionAccessFilteringActionBase::ShouldAnalyzeFunction(UBlueprint* Blueprint,
                                                                        UK2Node_FunctionEntry* FunctionEntry) const
{
    const bool bPublic = FUNC_Public & FunctionEntry->GetFunctionFlags();
    // ReSharper disable once CppTooWideScopeInitStatement
    const bool bProtected = FUNC_Protected & FunctionEntry->GetFunctionFlags();
    // ReSharper disable once CppTooWideScopeInitStatement
    const bool bPrivate = FUNC_Private & FunctionEntry->GetFunctionFlags();

    return bPublic || bCheckProtectedFunctions && bProtected || bCheckPrivateFunctions && bPrivate;
}
