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

#include "EnsureRequiredPropertiesPresent.h"

static const FName RequiredPropertyName("RuleRangerRequired");

void UEnsureRequiredPropertiesPresent::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    if (IsValid(Object))
    {
        const UBlueprint* Blueprint = Cast<UBlueprint>(Object);
        UClass* Class;
        if (Blueprint && Blueprint->GeneratedClass)
        {
            Class = Blueprint->GeneratedClass;
        }
        else
        {
            Class = Object->GetClass();
        }
        const FName Key("RuleRangerRequired");

        const UObject* DefaultObject = Class->GetDefaultObject(true);
        for (TFieldIterator<FProperty> PropertyIt(Class); PropertyIt; ++PropertyIt)
        {
            const FProperty* Property = *PropertyIt;
            if (PropertyIt->GetMetaDataMap() && PropertyIt->GetMetaDataMap()->Contains(Key))
            {
                if (const auto ObjectProperty = CastField<FObjectPropertyBase>(Property))
                {
                    if (1 == ObjectProperty->ArrayDim)
                    {
                        if (!ObjectProperty->GetObjectPropertyValue_InContainer(DefaultObject))
                        {
                            const auto& ErrorMessage = FString::Printf(
                                TEXT("Object contains property named '%s' that is not set but the property is "
                                     "annotated with the %s meta property which indicates it MUST be set."),
                                *Property->GetDisplayNameText().ToString(),
                                *RequiredPropertyName.ToString());
                            ActionContext->Error(FText::FromString(ErrorMessage));
                        }
                    }
                    else
                    {
                        for (int32 i = 0; i < ObjectProperty->ArrayDim; i++)
                        {
                            if (!ObjectProperty->GetObjectPropertyValue_InContainer(DefaultObject, i))
                            {
                                const auto& ErrorMessage = FString::Printf(
                                    TEXT("Object contains property named '%s' that has index %d that is not set "
                                         "but the property is annotated with the %s meta property which indicates "
                                         "it MUST be set."),
                                    *Property->GetDisplayNameText().ToString(),
                                    i,
                                    *RequiredPropertyName.ToString());
                                ActionContext->Error(FText::FromString(ErrorMessage));
                            }
                        }
                    }
                }
                else
                {
                    LogError(Object,
                             FString::Printf(TEXT("Property named %s is annotated with the meta property %s "
                                                  "but this is not supported property types other than references."),
                                             *Property->GetDisplayNameText().ToString(),
                                             *RequiredPropertyName.ToString()));
                }
            }
        }
    }
}
