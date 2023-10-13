// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterComponents/CombatComponent.h"
#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime,
                                     ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
    if (IsValid(Character) && IsValid(WeaponToEquip))
    {
        EquippedWeapon = WeaponToEquip;
        EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
        if (const auto HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket")))
        {
            HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
        }
        // Make sure we set owner of weapon so that weapon will be replicated wih player pawn
        EquippedWeapon->SetOwner(Character);
        EquippedWeapon->ShowPickupWidget(false);
    }
}
