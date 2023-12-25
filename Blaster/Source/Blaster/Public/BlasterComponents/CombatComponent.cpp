// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterComponents/CombatComponent.h"
#include "Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    MirrorWalkSpeedBasedOnState();
}

void UCombatComponent::MirrorWalkSpeedBasedOnState() const
{
    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;
    }
}

void UCombatComponent::SetAiming(bool bInAiming)
{
    // bAiming is set here because if this is called on client then we will locally set var before calling server
    // not needed on server as ServerSetAiming falls directly through to the server implementation
    bAiming = bInAiming;
    // Set the Aiming flag on the server
    ServerSetAiming(bInAiming);
    MirrorWalkSpeedBasedOnState();
}

void UCombatComponent::ServerSetAiming_Implementation(bool bInAiming)
{
    bAiming = bInAiming;
    // Make sure we set the WalkSpeed set on the server and replicated to other clients
    MirrorWalkSpeedBasedOnState();
}

void UCombatComponent::OnRep_EquippedWeapon()
{
    if (IsValid(EquippedWeapon) && IsValid(Character))
    {
        StopOrientingRotationToMovement();
    }
}

void UCombatComponent::SetFireButtonPressed(const bool bInFireButtonPressed)
{
    bFireButtonPressed = bInFireButtonPressed;
    if (bFireButtonPressed)
    {
        // Send fire action to the server
        ServerFire();
    }
}

void UCombatComponent::ServerFire_Implementation()
{
    // Send the fire action to all of the clients
    MulticastFire();
}

void UCombatComponent::MulticastFire_Implementation()
{
    if (IsValid(Character) && IsValid(EquippedWeapon))
    {
        Character->PlayFireMontage(bAiming);
        EquippedWeapon->Fire();
    }
}

void UCombatComponent::TickComponent(const float DeltaTime,
                                     const ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCombatComponent, EquippedWeapon);
    DOREPLIFETIME(UCombatComponent, bAiming);
}

static const FName RightHandSocketName("RightHandSocket");

void UCombatComponent::StopOrientingRotationToMovement() const
{
    Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
    if (IsValid(Character) && IsValid(WeaponToEquip))
    {
        EquippedWeapon = WeaponToEquip;
        EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
        if (const auto HandSocket = Character->GetMesh()->GetSocketByName(RightHandSocketName))
        {
            HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
        }
        // Make sure we set owner of weapon so that weapon will be replicated wih player pawn
        EquippedWeapon->SetOwner(Character);
        EquippedWeapon->ShowPickupWidget(false);

        StopOrientingRotationToMovement();
    }
}
