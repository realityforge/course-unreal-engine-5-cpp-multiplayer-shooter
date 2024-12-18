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
#include "EnsureMaterialHasNoCompileErrorAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureMaterialHasNoCompileErrorAction)

void UEnsureMaterialHasNoCompileErrorAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                  UObject* Object)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto Material = CastChecked<UMaterial>(Object);

    LogInfo(Material,
            FString::Printf(TEXT("Checking Material named '%s' compiles at FeatureLevel %s."),
                            *Material->GetName(),
                            *StaticEnum<EFeatureLevel>()->GetDisplayNameText().ToString()));

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto NativeFeatureLevel = EFeatureLevel::SM6 == FeatureLevel ? ERHIFeatureLevel::SM6
        : EFeatureLevel::SM5 == FeatureLevel                           ? ERHIFeatureLevel::SM5
                                                                       : ERHIFeatureLevel::ES3_1;

    if (Material->IsCompilingOrHadCompileError(NativeFeatureLevel))
    {
        if (0 == Material->GetExpressions().Num())
        {
            if (bErrorIfEmpty)
            {
                ActionContext->Error(FText::FromString(
                    TEXT("Material has 0 expressions and thus will not be compiled. Please rectify the error.")));
            }
            else
            {
                LogInfo(
                    Material,
                    FString::Printf(TEXT("Material has 0 expressions and thus will not be compiled. However the "
                                         "bErrorIfEmpty is set to false on the action  so ignoring this scenario.")));
            }
        }
        else
        {
            ActionContext->Error(
                FText::FromString(TEXT("Material is compiling or had a compile error. Please rectify the error.")));
        }
    }
    else
    {
        LogInfo(Material, FString::Printf(TEXT("Material has been compiled correctly.")));
    }
}

UClass* UEnsureMaterialHasNoCompileErrorAction::GetExpectedType()
{
    return UMaterial::StaticClass();
}
