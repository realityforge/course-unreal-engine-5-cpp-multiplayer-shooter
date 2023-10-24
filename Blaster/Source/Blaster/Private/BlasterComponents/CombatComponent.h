// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "CombatComponent.generated.h"

class AWeapon;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatComponent();

    virtual void
    TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;

    // As ABlasterCharacter is closely integrated with this component we made it a friend class to avoid excessive
    // public accessors etc
    friend class ABlasterCharacter;

    void EquipWeapon(AWeapon* WeaponToEquip);

protected:
    virtual void BeginPlay() override;

    void setAiming(bool bInAiming);

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool bInAiming);

    UFUNCTION()
    void OnRep_EquippedWeapon();

private:
    UPROPERTY(Transient)
    TObjectPtr<ABlasterCharacter> Character;

    /** The currently equipped weapon. */
    UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
    TObjectPtr<AWeapon> EquippedWeapon;

    /** Is the character currently aiming. */
    UPROPERTY(Replicated)
    bool bAiming{ false };

    void StopOrientingRotationToMovement() const;

public:
};
