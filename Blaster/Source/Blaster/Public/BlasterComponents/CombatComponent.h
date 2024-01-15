#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "HUD/BlasterHUD.h"
#include "CombatComponent.generated.h"

class ABlasterHUD;
class ABlasterPlayerController;
class ABlasterCharacter;
class AWeapon;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UCombatComponent final : public UActorComponent
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

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
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

    void TraceUnderCrossHairs(FHitResult& OutHitResult);

    void SetHUDCrosshairs(float DeltaTime);

private:
    UPROPERTY(Transient)
    TObjectPtr<ABlasterCharacter> Character{ nullptr };
    UPROPERTY(Transient)
    TObjectPtr<ABlasterPlayerController> Controller{ nullptr };
    UPROPERTY(Transient)
    TObjectPtr<ABlasterHUD> HUD{ nullptr };
    UPROPERTY(Transient)
    FHUDPackage HUDPackage;

    /** The currently equipped weapon. */
    UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
    TObjectPtr<AWeapon> EquippedWeapon{ nullptr };

    /** Is the character currently aiming. */
    UPROPERTY(Replicated)
    bool bAiming{ false };

    /** The speed at which the character moves when walking but not aiming. */
    UPROPERTY(EditAnywhere)
    float BaseWalkSpeed{ 600.f };

    /** The speed at which the character moves when walking and aiming. */
    UPROPERTY(EditAnywhere)
    float AimWalkSpeed{ 350.f };

    bool bFireButtonPressed{ false };

    /** The current spread factor because we are currently in air or have been recently in air */
    float CrosshairInAirFactor{ 0.f };

    /** The current spread factor because we are aiming or have been recently aiming */
    float CrosshairAimFactor{ 0.f };

    /** The current spread factor because we have recently shot. Accumulates with every shot. */
    float CrosshairShootingFactor{ 0.f };

    /**
     * The target hit by tracing from our crosshairs.
     * Done every frame for locally controlled characters to orient right hand when equipped with a weapon.
     * (Why not just do when weapon is equipped?)
     */
    FVector HitTarget;

    //---------------------------------------------------------------------------
    // Zoomed FOV while aiming
    //---------------------------------------------------------------------------

    /** The speed at which to interp back to default ZoomFOV. */
    UPROPERTY(EditAnywhere, Category = "Combat")
    float ZoomInterpSpeed{ 20.f };

    /** FOV when not aiming. (Cached from the cameras base FOV in BeginPlay) */
    float DefaultFOV{ 0 };

    /** Current FOV used during interping FOV */
    float CurrentFOV{ 0 };

    void InitDefaultFOV();

    void UpdateFOV(float DeltaTime);

    void StopOrientingRotationToMovement() const;

    //---------------------------------------------------------------------------
    // Automatic Fire Management
    //---------------------------------------------------------------------------

    void Fire();

    FTimerHandle FireTimer;
    bool bCanFire{ true };

    void StartFireTimer();
    void FireTimerFinished();
};
