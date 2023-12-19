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

#include "EnsureWeaponMeshHasSocket.h"
#include "RuleRanger/RuleRangerUtilities.h"
#include "Weapon/Weapon.h"

void UEnsureWeaponMeshHasSocket::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    if (IsValid(Object))
    {
        if (FRuleRangerUtilities::IsA(Object, AWeapon::StaticClass()))
        {
            if (const auto Blueprint = Cast<UBlueprint>(Object))
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                const AWeapon* DefaultObject =
                    Blueprint->GeneratedClass ? Blueprint->GeneratedClass->GetDefaultObject<AWeapon>() : nullptr;
                if (!DefaultObject || !DefaultObject->GetWeaponMesh()->DoesSocketExist(FName("LeftHandSocket")))
                {
                    ActionContext->Error(FText::FromString(
                        "Socket LeftHandSocket does not exist on mesh assigned to the WeaponMesh component"));
                }
            }
            else if (const auto Weapon = Cast<AWeapon>(Object))
            {
                if (Weapon->GetWeaponMesh()->DoesSocketExist(FName("LeftHandSocket")))
                {
                    ActionContext->Error(FText::FromString(
                        "Socket LeftHandSocket does not exist on mesh assigned to the WeaponMesh component"));
                }
            }
            else
            {
                LogError(Object, TEXT("Conrete subclasses of Weapon expected to be Blueprints"));
            }
        }
        else
        {
            LogError(Object, TEXT("Action running on class that is not a Weapon"));
        }
    }
}
