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
#include "RuleRanger/Matchers/EditorPropertyMatcher.h"
#include "Editor.h"
#include "Kismet/KismetStringLibrary.h"
#include "RuleRanger/RuleRangerUtilities.h"

bool UEditorPropertyMatcher::Test_Implementation(UObject* Object)
{
    if (Name != NAME_None && IsValid(Object))
    {
        TArray<UObject*> Instances;
        RuleRangerUtilities::CollectInstanceHierarchy(Object, Instances);

        for (const auto Instance : Instances)
        {
            TArray<UClass*> Classes;
            RuleRangerUtilities::CollectTypeHierarchy(Instance, Classes);
            for (const auto Class : Classes)
            {
                if (const auto Property = Class->FindPropertyByName(Name))
                {
                    if (const auto BoolProperty = CastField<const FBoolProperty>(Property))
                    {
                        return BoolProperty->GetPropertyValue(Instance)
                            == Value.Equals("true", ESearchCase::IgnoreCase);
                    }
                    else if (const auto EnumProperty = CastField<const FEnumProperty>(Property))
                    {
                        const UEnum* Enum = EnumProperty->GetEnum();
                        const int EnumValue = *EnumProperty->ContainerPtrToValuePtr<uint8>(Instance);
                        const int32 EnumIndex = Enum->GetIndexByValue(EnumValue);
                        const FName EnumName = Enum->GetNameByIndex(EnumIndex);
                        const FString EnumAuthoredName = Enum->GetAuthoredNameStringByIndex(EnumIndex);
                        const FText EnumDisplayName = Enum->GetDisplayNameTextByIndex(EnumIndex);
                        return EnumName.ToString().Equals(Value) || EnumAuthoredName.Equals(Value)
                            || EnumDisplayName.ToString().Equals(Value);
                    }
                    else if (const auto NumericProperty = CastField<FNumericProperty>(Property))
                    {
                        if (NumericProperty->IsEnum())
                        {
                            const UEnum* Enum = NumericProperty->GetIntPropertyEnum();
                            const int EnumValue = *NumericProperty->ContainerPtrToValuePtr<uint8>(Instance);
                            const int32 EnumIndex = Enum->GetIndexByValue(EnumValue);
                            const FName EnumName = Enum->GetNameByIndex(EnumIndex);
                            const FString EnumAuthoredName = Enum->GetAuthoredNameStringByIndex(EnumIndex);
                            const FText EnumDisplayName = Enum->GetDisplayNameTextByIndex(EnumIndex);
                            return EnumName.ToString().Equals(Value) || EnumAuthoredName.Equals(Value)
                                || EnumDisplayName.ToString().Equals(Value);
                        }
                        else if (NumericProperty->IsInteger())
                        {
                            return NumericProperty->GetSignedIntPropertyValue(Instance)
                                == UKismetStringLibrary::Conv_StringToInt64(Value);
                        }
                        else if (NumericProperty->IsFloatingPoint())
                        {
                            return NumericProperty->GetFloatingPointPropertyValue(Instance)
                                == UKismetStringLibrary::Conv_StringToDouble(Value);
                        }
                    }
                }
            }
        }
    }
    return false;
}
