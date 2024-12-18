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
#include "EnsureRequiredPropertiesPresentAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureRequiredPropertiesPresentAction)

static const FName RequiredPropertyName("RuleRangerRequired");

void UEnsureRequiredPropertiesPresentAction::PerformChecksOnObject(URuleRangerActionContext* ActionContext,
                                                                   const UObject* Object) const
{
    for (TFieldIterator<FProperty> PropertyIt(Object->GetClass()); PropertyIt; ++PropertyIt)
    {
        const FProperty* Property = *PropertyIt;
        if (PropertyIt->GetMetaDataMap() && PropertyIt->GetMetaDataMap()->Contains(RequiredPropertyName))
        {
            if (const auto ObjectProperty = CastField<FObjectPropertyBase>(Property))
            {
                if (1 == ObjectProperty->ArrayDim)
                {
                    if (!ObjectProperty->LoadObjectPropertyValue_InContainer(Object))
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
                        if (!ObjectProperty->LoadObjectPropertyValue_InContainer(Object, i))
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

void UEnsureRequiredPropertiesPresentAction::DeriveRequiredComponentProperties(
    const UClass* Class,
    TMap<FString, TArray<FString>>& RequiredComponentProperties)
{
    for (const UStruct* TestStruct = Class; TestStruct; TestStruct = TestStruct->GetSuperStruct())
    {
        if (const FString* Value = TestStruct->FindMetaData(RequiredPropertyName))
        {
            TArray<FString> Rules;
            Value->ParseIntoArray(Rules, TEXT(","));
            for (auto RuleIt = Rules.CreateConstIterator(); RuleIt; ++RuleIt)
            {
                const auto Rule = *RuleIt;
                FString ComponentName;
                FString PropertyName;
                if (Rule.Split(TEXT("."), &ComponentName, &PropertyName))
                {
                    // Only add require rule for component properties
                    if (RequiredComponentProperties.Contains(ComponentName))
                    {
                        RequiredComponentProperties.FindChecked(ComponentName).Add(PropertyName);
                    }
                    else
                    {
                        TArray<FString> Values;
                        Values.Add(PropertyName);
                        RequiredComponentProperties.Add(ComponentName, Values);
                    }
                }
            }
        }
    }
}

void UEnsureRequiredPropertiesPresentAction::PerformChecksForComponentProperties(
    URuleRangerActionContext* ActionContext,
    const UClass* Class,
    const FString& ComponentName,
    const UObject* Component,
    const TArray<FString>& PropertyNames)
{
    for (auto PropertyNameIt = PropertyNames.CreateConstIterator(); PropertyNameIt; ++PropertyNameIt)
    {
        const auto PropertyName = *PropertyNameIt;
        if (const FProperty* Property = Component->GetClass()->FindPropertyByName(FName(PropertyName)))
        {
            if (const auto ObjectProperty = CastField<FObjectPropertyBase>(Property))
            {
                if (1 == ObjectProperty->ArrayDim)
                {
                    if (!ObjectProperty->LoadObjectPropertyValue_InContainer(Component))
                    {
                        const auto& ErrorMessage =
                            FString::Printf(TEXT("The %s class (or a parent class) declared that the "
                                                 "property %s must be present on the sub-object named %s "
                                                 "and the property is present but value is not set."),
                                            *Class->GetAuthoredName(),
                                            *PropertyName,
                                            *ComponentName);
                        ActionContext->Error(FText::FromString(ErrorMessage));
                    }
                }
                else
                {
                    for (int32 i = 0; i < ObjectProperty->ArrayDim; i++)
                    {
                        if (!ObjectProperty->LoadObjectPropertyValue_InContainer(Component, i))
                        {
                            const auto& ErrorMessage =
                                FString::Printf(TEXT("The %s class (or a parent class) declared that the "
                                                     "property %s must be present on the sub-object named %s "
                                                     "and the property is present but value is not set at index %d."),
                                                *Class->GetAuthoredName(),
                                                *PropertyName,
                                                *ComponentName,
                                                i);
                            ActionContext->Error(FText::FromString(ErrorMessage));
                        }
                    }
                }
            }
            else
            {
                const auto& ErrorMessage =
                    FString::Printf(TEXT("The %s class (or a parent class) declared that the "
                                         "property %s must be present on the sub-object named %s "
                                         "and the property is present on the sub-object but the "
                                         "property is not a valid object reference."),
                                    *Class->GetAuthoredName(),
                                    *PropertyName,
                                    *ComponentName);
                ActionContext->Error(FText::FromString(ErrorMessage));
            }
        }
        else
        {
            const auto& ErrorMessage =
                FString::Printf(TEXT("The %s class (or a parent class) declared that the property %s must "
                                     "be present on the sub-object named %s "
                                     "and the property is not present on the sub-object."),
                                *Class->GetAuthoredName(),
                                *PropertyName,
                                *ComponentName);
            ActionContext->Error(FText::FromString(ErrorMessage));
        }
    }
}

void UEnsureRequiredPropertiesPresentAction::PerformChecksForComponentProperties(
    URuleRangerActionContext* ActionContext,
    const UObject* Object,
    const TMap<FString, TArray<FString>>& RequiredComponentProperties)
{
    const UClass* Class = Object->GetClass();
    for (auto EntryIt = RequiredComponentProperties.CreateConstIterator(); EntryIt; ++EntryIt)
    {
        const auto ComponentName = EntryIt->Key;
        const auto PropertyNames = EntryIt->Value;
        if (const FProperty* Property = Class->FindPropertyByName(FName(ComponentName)))
        {
            if (const auto ComponentProperty = CastField<FObjectPropertyBase>(Property))
            {
                if (1 == ComponentProperty->ArrayDim)
                {
                    if (const UObject* Component = ComponentProperty->LoadObjectPropertyValue_InContainer(Object))
                    {
                        PerformChecksForComponentProperties(ActionContext,
                                                            Class,
                                                            ComponentName,
                                                            Component,
                                                            PropertyNames);
                    }
                    else
                    {
                        // TODO: We could add configuration, that optionally ignored null sub-object references
                        const auto& ErrorMessage =
                            FString::Printf(TEXT("The %s class (or a parent class) declared that the "
                                                 "properties [%s] must be present on the sub-object named %s "
                                                 "but the sub-object property is null."),
                                            *Class->GetAuthoredName(),
                                            *FString::Join(PropertyNames, TEXT(",")),
                                            *ComponentName);
                        ActionContext->Error(FText::FromString(ErrorMessage));
                    }
                }
                else
                {
                    // TODO: We could support this in the future but will not bother until there is a need
                    const auto& ErrorMessage =
                        FString::Printf(TEXT("The %s class (or a parent class) declared that the "
                                             "properties [%s] must be present on the sub-object named %s "
                                             "but the sub-object property is an array which is not supported."),
                                        *Class->GetAuthoredName(),
                                        *FString::Join(PropertyNames, TEXT(",")),
                                        *ComponentName);
                    ActionContext->Error(FText::FromString(ErrorMessage));
                }
            }
            else
            {
                const auto& ErrorMessage =
                    FString::Printf(TEXT("The %s class (or a parent class) declared that the "
                                         "properties [%s] must be present on the sub-object named %s "
                                         "but the sub-object property is not a valid object reference."),
                                    *Class->GetAuthoredName(),
                                    *FString::Join(PropertyNames, TEXT(",")),
                                    *ComponentName);
                ActionContext->Error(FText::FromString(ErrorMessage));
            }
        }
        else
        {
            const auto& ErrorMessage =
                FString::Printf(TEXT("The %s class (or a parent class) declared that the properties [%s] must "
                                     "be present on the sub-object named %s but no such sub-object property "
                                     "was defined for the class."),
                                *Class->GetAuthoredName(),
                                *FString::Join(PropertyNames, TEXT(",")),
                                *ComponentName);
            ActionContext->Error(FText::FromString(ErrorMessage));
        }
    }
}

void UEnsureRequiredPropertiesPresentAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                  UObject* Object)
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

    if (Class->HasAnyClassFlags(CLASS_Abstract) && bSkipCheckOnAbstractTypes)
    {
        return;
    }

    // A map of ComponentName -> [PropertyName] of properties that are required
    TMap<FString, TArray<FString>> RequiredComponentProperties;
    DeriveRequiredComponentProperties(Class, RequiredComponentProperties);

    const UObject* DefaultObject = Class->GetDefaultObject(true);

    if (!RequiredComponentProperties.IsEmpty())
    {
        PerformChecksForComponentProperties(ActionContext, DefaultObject, RequiredComponentProperties);
    }

    // Perform checks for properties directly on the object
    PerformChecksOnObject(ActionContext, DefaultObject);
}
