#include "Character/BlasterAnimInstance.h"
#include "Character/BlasterCharacter.h"
#include "Character/CombatState.h"
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
        bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
        bEliminated = BlasterCharacter->IsEliminated();

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

            if (BlasterCharacter->IsLocallyControlled())
            {
                bLocallyControlled = true;
                const auto RightHandTransform = WeaponMesh->GetSocketTransform(RightHandBoneName, RTS_World);
                const auto HitTarget = BlasterCharacter->GetHitTarget();

                // Right hand bone has x axis pointing up to elbow rather than pointing down to fingers

                const FVector Target =
                    RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - HitTarget);
                const auto TargetRightHandRotation =
                    UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), Target);
                // Interp the rotation change if any so that moving from aiming at a far target to near target will  not
                // result in a jerk but a smooth interpolation
                RightHandRotation = FMath::RInterpTo(RightHandRotation, TargetRightHandRotation, DeltaSeconds, 30.f);

                // DEBUG: Shows the difference lines between muzzle and where we are actually aiming and gun orientation
                //
                // const FTransform MuzzleFlashTransform = WeaponMesh->GetSocketTransform(FName("MuzzleFlash"),
                // RTS_World); const FVector ForwardVector = MuzzleFlashTransform.GetRotation().GetForwardVector();
                // DrawDebugLine(GetWorld(),
                //               MuzzleFlashTransform.GetLocation(),
                //               MuzzleFlashTransform.GetLocation() + 1000.f * ForwardVector,
                //               FColor::Emerald);
                // DrawDebugLine(GetWorld(), MuzzleFlashTransform.GetLocation(), HitTarget, FColor::Orange);
            }
        }
        // We skip FABRIK when reloading as the montage already positions the hand correctly
        bUseFABRIK = ECombatState::Reloading != BlasterCharacter->GetCombatState();
        const bool bDisableGameplay = BlasterCharacter->GetDisableGameplay();
        // Skip aim offsets when reloading otherwise hand will not pull mag from correct location
        // also skip if gameplay is disabled because we can move our view around but character should
        // not change where they are aiming
        bUseAimOffsets = ECombatState::Reloading != BlasterCharacter->GetCombatState() && !bDisableGameplay;
        bTransformRightHand = ECombatState::Reloading != BlasterCharacter->GetCombatState() && !bDisableGameplay;
    }
}
