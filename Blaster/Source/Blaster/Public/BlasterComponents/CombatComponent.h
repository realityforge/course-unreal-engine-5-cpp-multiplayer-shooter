// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "CombatComponent.generated.h"

class ABlasterCharacter;
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
    void MirrorWalkSpeedBasedOnState() const;

    void SetAiming(bool bInAiming);

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool bInAiming);

    UFUNCTION()
    void OnRep_EquippedWeapon();

    void SetFireButtonPressed(bool bInFireButtonPressed);

    UFUNCTION(Server, Reliable)
    void ServerFire(const FVector_NetQuantize& TraceHitTarget);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

    void TraceUnderCrossHairs(FHitResult& OutHitResult) const;

private:
    UPROPERTY(Transient)
    TObjectPtr<ABlasterCharacter> Character;

    /** The currently equipped weapon. */
    UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
    TObjectPtr<AWeapon> EquippedWeapon;

    /** Is the character currently aiming. */
    UPROPERTY(Replicated)
    bool bAiming{ false };

    /** The speed at which the character moves when walking but not aiming. */
    UPROPERTY(EditAnywhere)
    float BaseWalkSpeed{ 600.f };

    /** The speed at which the character moves when walking and aiming. */
    UPROPERTY(EditAnywhere)
    float AimWalkSpeed{ 450.f };

    bool bFireButtonPressed{ false };

    void StopOrientingRotationToMovement() const;
};
