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
#include "EnsureDataOnlyBlueprintAction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "RuleRanger/RuleRangerUtilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureDataOnlyBlueprintAction)

void UEnsureDataOnlyBlueprintAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    TSubclassOf<UObject> MatchedObjectType{ nullptr };
    for (const auto ObjectType : ObjectTypes)
    {
        if (FRuleRangerUtilities::IsA(Object, ObjectType))
        {
            MatchedObjectType = ObjectType;
        }
    }
    // ReSharper disable once CppTooWideScopeInitStatement
    const UBlueprint* Blueprint = CastChecked<UBlueprint>(Object);
    if (MatchedObjectType && !FBlueprintEditorUtils::IsDataOnlyBlueprint(Blueprint))
    {
        ActionContext->Error(FText::FromString(
            FString::Printf(TEXT("Object extended type %s and is expected to be a DataOnlyBlueprint but is not"),
                            *MatchedObjectType->GetOutermost()->GetName())));
    }
}

UClass* UEnsureDataOnlyBlueprintAction::GetExpectedType()
{
    return UBlueprint::StaticClass();
}
