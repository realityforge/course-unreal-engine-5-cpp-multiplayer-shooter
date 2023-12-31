#include "Character/BlasterAnimInstance.h"
#include "Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Weapon.h"

// The name of the socket that must be on every weapon so that they have IK and LH links up
static const FName LeftHandSocketName("LeftHandSocket");

// The name of the bone that holds the weapon
static const FName RightHandBoneName("hand_r");

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
    if (IsValid(BlasterCharacter))
    {
        FVector Velocity = BlasterCharacter->GetVelocity();
        // We are only looking for lateral Velocity so we zero out height
        Velocity.Z = 0.f;
        Speed = Velocity.Size();

        bInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

        // Current acceleration vector is calculated each update based on input vector so we are calculating whether
        // we are receiving movement input this update and could rename the flag... but leaving to align with course
        bAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;

        bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
        EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
        bIsCrouched = BlasterCharacter->bIsCrouched;
        bAiming = BlasterCharacter->IsAiming();
        AimOffsetYaw = BlasterCharacter->GetAimOffsetYaw();
        AimOffsetPitch = BlasterCharacter->GetAimOffsetPitch();
        TurningInPlace = BlasterCharacter->GetTurningInPlace();

        // AimRotation is in world space
        const FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();

        // X is facing forwards for character so this gets the direction of forward movement (i.e. Velocity),
        // gets its X direction and converts it into a Rotator. X is also in world space
        const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());

        // Calculate the offset between the direction the character is moving and the direction they are facing
        const FRotator TargetDeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
        // Note that we interp it here rather than in the blueprint because the math code here is smart enough to interp
        // -180 to 180 smoothly (i.e. when character is facing forwards and moving backwards. The blendspace interp
        // would use linear interp which would rotate -180->180 (going through 0) which would lead to glitchiness in the
        // animation.
        DeltaRotation = FMath::RInterpTo(DeltaRotation, TargetDeltaRotation, DeltaSeconds, 6.f);
        YawOffset = DeltaRotation.Yaw;

        YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

        // YawOffset will be used for strafing

        // Lean is calculated from delta between rotation last frame and current frame
        CharacterRotationLastFrame = CharacterRotation;
        CharacterRotation = BlasterCharacter->GetActorRotation();

        const FRotator Delta =
            UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);

        // Calculate the rotation we should target based on movement
        const float TargetYawDelta = Delta.Yaw / DeltaSeconds;

        // Smooth it out to avoid jerkiness
        const float TargetYawDeltaInterp = FMath::FInterpTo(Lean, TargetYawDelta, DeltaSeconds, 6.f);

        // Clamp it to a reasonable range
        Lean = FMath::Clamp(TargetYawDeltaInterp, -90.f, 90.f);

        const auto CharacterMesh = BlasterCharacter->GetMesh();
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto WeaponMesh = EquippedWeapon ? EquippedWeapon->GetWeaponMesh() : nullptr;
        if (CharacterMesh && WeaponMesh)
        {
            // get the location of the LeftHandSocket sock in world space
            LeftHandTransform = WeaponMesh->GetSocketTransform(LeftHandSocketName, RTS_World);

            FVector OutPosition;
            FRotator OutRotation;

            // Using right hand bone as reference (to accidentally avoid moving the weapon relative to right hand)
            // Transform the location/rotation from world space to bone relative space.
            CharacterMesh->TransformToBoneSpace(
                RightHandBoneName,
                LeftHandTransform.GetLocation(),
                // NOTE: Coursework used FRotator::ZeroRotator but this means we could not setup orientation correctly
                LeftHandTransform.GetRotation().Rotator(),
                OutPosition,
                OutRotation);
            LeftHandTransform.SetLocation(OutPosition);
            LeftHandTransform.SetRotation(FQuat(OutRotation));
        }
    }
}
