#include "Character/BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    if (!BlasterCharacter)
    {
        // BlasterCharacter can be null when run in editor
        BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
    }
    if (BlasterCharacter)
    {
        FVector Velocity = BlasterCharacter->GetVelocity();
        // We are only looking for lateral Velocity so we zero out height
        Velocity.Z = 0.f;
        Speed = Velocity.Size();

        bInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

        // Current acceleration vector is calculated each update based on input vector so we are calculating whether
        // we are receiving movement input this update and could rename the flag... but leaving to align with course
        bAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
    }
}
