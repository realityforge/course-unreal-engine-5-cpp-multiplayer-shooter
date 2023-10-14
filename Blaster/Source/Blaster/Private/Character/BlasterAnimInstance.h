#pragma once

#include "Animation/AnimInstance.h"
#include "CoreMinimal.h"
#include "BlasterAnimInstance.generated.h"

class ABlasterCharacter;

UCLASS()
class UBlasterAnimInstance : public UAnimInstance
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

    /** True if the character is currently crouching. */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouched{ false };
};
