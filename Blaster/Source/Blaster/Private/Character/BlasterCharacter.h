#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "TurningInPlace.h"
#include "BlasterCharacter.generated.h"

class UCombatComponent;
class AWeapon;
class UWidgetComponent;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class ABlasterCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABlasterCharacter();

    virtual void Tick(float DeltaTime) override;

    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;

    virtual void PostInitializeComponents() override;

protected:
    virtual void BeginPlay() override;

    //---------------------------------------------------------------------------
    // User Inputs Section
    //---------------------------------------------------------------------------

    /** Called for movement input */
    void MoveInputActionTriggered(const FInputActionValue& Value);

    /** Called for looking input */
    void LookInputActionTriggered(const FInputActionValue& Value);

    /** Called for attempting to equip an item */
    void EquipInputActionTriggered();

    /** Called when player start to crouch */
    void OnCrouchInputActionStarted();

    /** Called when player releases key to crouch */
    void OnCrouchInputActionCompleted();

    /** Called when player starts to aim */
    void OnAimInputActionStarted();

    /** Called when player releases key to aim */
    void OnAimInputActionCompleted();

    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Animation State Section
    //---------------------------------------------------------------------------

    void CalculateAimOffset(float DeltaTime);

    void CalculateTurnInPlace(float DeltaTime);

    //---------------------------------------------------------------------------

private:
    //---------------------------------------------------------------------------
    // User Inputs Section
    //---------------------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "Character Input", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<UInputMappingContext> InputMapping{ nullptr };

    /** Jump Input Action */
    UPROPERTY(EditDefaultsOnly, Category = "Character Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> JumpAction{ nullptr };

    /** Move Input Action */
    UPROPERTY(EditDefaultsOnly, Category = "Character Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> MoveAction{ nullptr };

    /** Look Input Action */
    UPROPERTY(EditDefaultsOnly, Category = "Character Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> LookAction{ nullptr };

    /** Equip Input Action */
    UPROPERTY(EditDefaultsOnly, Category = "Character Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> EquipAction{ nullptr };

    /** Crouch Input Action */
    UPROPERTY(EditDefaultsOnly, Category = "Character Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> CrouchAction{ nullptr };

    /** Crouch Input Action */
    UPROPERTY(EditDefaultsOnly, Category = "Character Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> AimAction{ nullptr };

    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Third Person Camera Section
    //---------------------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    TObjectPtr<USpringArmComponent> CameraBoom{ nullptr };

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    TObjectPtr<UCameraComponent> FollowCamera{ nullptr };

    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // HUD View
    //---------------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UWidgetComponent> OverheadWidget{ nullptr };

    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Animation State Section
    //---------------------------------------------------------------------------

    float AimOffsetYaw{ 0.f };
    float AimOffsetPitch{ 0.f };
    // This is the rotation when the character last stopped moving or turned in place and is
    // used when calculating aim offset yaw/pitch of character when standing still
    FRotator AimOffsetBaseAimRotation;

    ETurningInPlace TurningInPlace{ ETurningInPlace::TIP_NotTurning };

    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Weapon Management
    //---------------------------------------------------------------------------

    // The reference to the weapon we are overlapping
    UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
    TObjectPtr<AWeapon> OverlappingWeapon;

    UFUNCTION()
    void OnRep_OverlappingWeapon(AWeapon* OldOverlappingWeapon) const;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    TObjectPtr<UCombatComponent> Combat{ nullptr };

    UFUNCTION(Server, Reliable)
    void ServerEquip();

    void ShowPickupWidgetOnOverlappingWeapon(bool bShowWidget) const;

    //---------------------------------------------------------------------------
    void SafeBindAction(UEnhancedInputComponent* const Input,
                        const TCHAR* Label,
                        const TObjectPtr<UInputAction> InputAction,
                        const ETriggerEvent TriggerEvent,
                        void (ThisClass::*Func)());

    void SafeBindAction(UEnhancedInputComponent* Input,
                        const TCHAR* Label,
                        TObjectPtr<UInputAction> InputAction,
                        ETriggerEvent TriggerEvent,
                        void (ThisClass::*Func)(const FInputActionValue&));

public:
    /** Return the CameraBoom SubObject **/
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    /** Return the FollowCamera SubObject **/
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    FORCEINLINE float GetAimOffsetYaw() const { return AimOffsetYaw; }
    FORCEINLINE float GetAimOffsetPitch() const { return AimOffsetPitch; }
    FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

    void SetOverlappingWeapon(AWeapon* Weapon);

    /** Return the EquippedWeapon if any. **/
    AWeapon* GetEquippedWeapon() const;

    bool IsWeaponEquipped() const;
    bool IsAiming() const;
};
