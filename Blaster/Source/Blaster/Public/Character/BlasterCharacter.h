#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "TurningInPlace.h"
#include "interfaces/InterfaceWithCrosshair.h"
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
class ABlasterCharacter : public ACharacter, public IInterfaceWithCrosshair
{
    GENERATED_BODY()

public:
    ABlasterCharacter();

    virtual void Tick(float DeltaTime) override;

    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;

    virtual void PostInitializeComponents() override;

    void PlayFireMontage(bool bAiming);

    virtual void OnRep_ReplicatedMovement() override;

    UFUNCTION(NetMulticast, Reliable)
    void Eliminate();

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

    /** Called when player presses the fire button */
    void OnFireInputActionStarted();

    /** Called when player releases the fire button */
    void OnFireInputActionCompleted();

    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Character Movement Overrides Section
    //---------------------------------------------------------------------------

    virtual void Jump() override;

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

    /** Fire Input Action */
    UPROPERTY(EditDefaultsOnly, Category = "Character Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> FireAction{ nullptr };

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
    float AimOffsetYawInterp{ 0.f };

    // This is the rotation when the character last stopped moving or turned in place and is
    // used when calculating aim offset yaw/pitch of character when standing still
    FRotator AimOffsetBaseAimRotation;

    ETurningInPlace TurningInPlace{ ETurningInPlace::TIP_NotTurning };

    UPROPERTY(EditAnywhere, Category = Combat, meta = (RuleRangerRequired))
    UAnimMontage* FireWeaponMontage{ nullptr };

    UPROPERTY(EditAnywhere, Category = Combat, meta = (RuleRangerRequired))
    UAnimMontage* HitReactMontage{ nullptr };

    void PlayHitReactMontage() const;

    UPROPERTY(EditAnywhere, Category = Combat, meta = (RuleRangerRequired))
    UAnimMontage* EliminationMontage{ nullptr };

    void PlayEliminationMontage() const;

    void CalculateAimOffsetPitch();

    void SimProxiesTurn();

    /**
     * Should we rotate the root bone?
     * We should be rotating root bone if not a simulated proxy.
     * (Locally controlled on server or client)
     */
    bool bRotateRootBone{ false };

    float ProxyTurnThreshold = 0.5f;

    FRotator ProxyRotationLastFrame;

    float TimeSinceLastMovementReplication{ 0.f };

    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Health Management
    //---------------------------------------------------------------------------

    UPROPERTY(EditAnywhere, Category = "Player Stats")
    float MaxHealth{ 100.f };

    UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Health, Category = "Player Stats")
    float Health{ 100.f };

    UPROPERTY(Transient, VisibleInstanceOnly)
    bool bEliminated;

    UFUNCTION()
    void OnRep_Health();

    void UpdateHUDHealth() const;

    UFUNCTION()
    void OnTakeDamage(AActor* DamagedActor,
                      float Damage,
                      const UDamageType* DamageType,
                      AController* InstigatorController,
                      AActor* DamageCauser);
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

    void HideCharacterIfCameraClose() const;

    /** The distance from the camera where the camera will disappear */
    UPROPERTY(EditDefaultsOnly)
    float CameraThreshold = 200.f;

    float CalculateSpeed() const;

public:
    /** Return the CameraBoom SubObject **/
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    /** Return the FollowCamera SubObject **/
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    FORCEINLINE float GetAimOffsetYaw() const { return AimOffsetYaw; }
    FORCEINLINE float GetAimOffsetPitch() const { return AimOffsetPitch; }
    FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
    FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
    FORCEINLINE bool IsEliminated() const { return bEliminated; }

    void SetOverlappingWeapon(AWeapon* Weapon);

    /** Return the EquippedWeapon if any. **/
    AWeapon* GetEquippedWeapon() const;

    bool IsWeaponEquipped() const;
    bool IsAiming() const;

    FVector GetHitTarget() const;
};
