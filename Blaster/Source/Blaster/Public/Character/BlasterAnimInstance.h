#pragma once

#include "Animation/AnimInstance.h"
#include "CoreMinimal.h"
#include "TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"

class AWeapon;
class ABlasterCharacter;

UCLASS()
class UBlasterAnimInstance final : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
    /** The Character that the AnimInstance is animating */
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    ABlasterCharacter* BlasterCharacter{ nullptr };

    /** The speed at which the character is moving. */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Speed{ 0.f };

    /** True if the character is in the air. */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bInAir{ false };

    /** True if the character is accelerating. */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bAccelerating{ false };

    /** True if the character has a weapon currently equipped. */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bWeaponEquipped{ false };

    /** Reference to the weapon currently equipped. */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AWeapon> EquippedWeapon{ nullptr };

    /** True if the character is currently crouching. */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouched{ false };

    /** True if the character is currently Aiming. */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bAiming{ false };

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float YawOffset{ 0.f };

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Lean{ 0.f };

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float AimOffsetYaw{ 0.f };

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float AimOffsetPitch{ 0.f };

    // This is the target for IK. It is effectively where we put the hand on the weapon
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    FTransform LeftHandTransform;

    // Lean is derived from current rotation and last frames rotation
    FRotator CharacterRotationLastFrame;
    FRotator CharacterRotation;
    FRotator DeltaRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    ETurningInPlace TurningInPlace{ ETurningInPlace::TIP_NotTurning };

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    FRotator RightHandRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bLocallyControlled{ false };

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bRotateRootBone{ false };

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bEliminated{ false };

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bUseFABRIK{ false };

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bUseAimOffsets{ false };

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bTransformRightHand{ false };
};
