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

    // As ABlasterCharacter is closely integrated with this component we made it a friend class to avoid excessive
    // public accessors etc
    friend class ABlasterCharacter;

    void EquipWeapon(AWeapon* WeaponToEquip);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<ABlasterCharacter> Character;
    UPROPERTY(Transient)
    TObjectPtr<AWeapon> EquippedWeapon;

public:
};
