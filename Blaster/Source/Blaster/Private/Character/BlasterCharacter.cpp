#include "Character/BlasterCharacter.h"
#include "Blaster/Blaster.h"
#include "BlasterComponents/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameMode/BlasterGameMode.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Sound/SoundCue.h"
#include "Weapon/Weapon.h"

ABlasterCharacter::ABlasterCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Make sure we always spawn the character, so can not fail to respawn.
    SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
    // We attach to the mesh rather than the root component. Because when we
    // crouch the root capsule will change which would mean this boom changed
    // position which is not something we want.
    CameraBoom->SetupAttachment(GetMesh());
    CameraBoom->TargetArmLength = 600.f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>("FollowCamera");
    // We attach to the socket at the end of the boom that has a constant name
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

    // Make sure the rotation from controller drives the boom rotation and not the camera
    CameraBoom->bUsePawnControlRotation = true;
    FollowCamera->bUsePawnControlRotation = false;

    // Setup the Follow camera so that it is always sharp, regardless of where you are viewing
    // This is most needed when zooming as you can have a high FOV and view close or far objects
    // and you want both to be sharp
    FollowCamera->PostProcessSettings.bOverride_DepthOfFieldFstop = true;
    FollowCamera->PostProcessSettings.DepthOfFieldFstop = 32;
    FollowCamera->PostProcessSettings.bOverride_DepthOfFieldFocalDistance = true;
    FollowCamera->PostProcessSettings.DepthOfFieldFocalDistance = 10000;

    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;

    OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
    OverheadWidget->SetupAttachment(GetMesh());

    Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
    Combat->SetIsReplicated(true);

    // The CharacterMovement component already supports crouching - we just need to enable it.
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

    // Speed up rotation rate to make it feel more responsive while still being smooth (was z=360)
    GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

    // Make sure when another character moves between our camera and our character, the camera boom
    // does not move in. We do this by making sure we do not block camera
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
    GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    // Set some values for update that are relatively high as the character is the actor
    // most of interest in a fast moving FPS
    NetUpdateFrequency = 66.f;
    MinNetUpdateFrequency = 33.f;
    NetPriority = 5;

    DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void ABlasterCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
    Super::EndPlay(EndPlayReason);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
    DOREPLIFETIME(ABlasterCharacter, Health);
}

void ABlasterCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    if (IsValid(Combat))
    {
        Combat->Character = this;
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::PlayFireMontage(const bool bAiming)
{
    if (IsValid(Combat) && IsValid(Combat->EquippedWeapon) && IsValid(FireWeaponMontage))
    {
        check(GetMesh());
        if (const auto AnimInstance = GetMesh()->GetAnimInstance(); IsValid(AnimInstance))
        {
            AnimInstance->Montage_Play(FireWeaponMontage);
            // Animation montage should be configured based on weapon ...
            const FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
            AnimInstance->Montage_JumpToSection(SectionName);
        }
    }
}

void ABlasterCharacter::PlayHitReactMontage() const
{
    check(Combat);
    if (IsValid(Combat) && IsValid(Combat->EquippedWeapon) && IsValid(HitReactMontage))
    {
        check(GetMesh());
        if (const auto AnimInstance = GetMesh()->GetAnimInstance(); IsValid(AnimInstance))
        {
            AnimInstance->Montage_Play(HitReactMontage);
            // Animation montage should be configured based on hit direction ...
            const FName SectionName("FromFront");
            AnimInstance->Montage_JumpToSection(SectionName);
        }
    }
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
    Super::OnRep_ReplicatedMovement();

    SimProxiesTurn();

    // In tick we fake network replication and call this method thus we reset the timer here
    // so we don't have tick pass through here
    TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::Eliminate()
{
    // Drop Weapon
    if (Combat && Combat->EquippedWeapon)
    {
        Combat->EquippedWeapon->Dropped();
    }
    MulticastEliminate();
    GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &ABlasterCharacter::RespawnTimerFinished, RespawnDelay);
}

void ABlasterCharacter::DisableCharacterMovement()
{
    // Disables movement of the character
    GetCharacterMovement()->DisableMovement();
    // Disables rotation of character
    GetCharacterMovement()->StopMovementImmediately();
    // Disables input (so can not keep firing
    if (const auto PlayerController = Cast<APlayerController>(Controller))
    {
        DisableInput(PlayerController);
    }
}

void ABlasterCharacter::DisableCollision() const
{
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABlasterCharacter::SpawnEliminationEffect()
{
    const auto& Location = GetActorLocation();
    // Spawn the effect/sound just above the character
    const FVector SpawnPoint(Location.X, Location.Y, Location.Z + 200.f);
    if (EliminationBotEffect)
    {
        EliminationBotComponent =
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EliminationBotEffect, SpawnPoint, GetActorRotation());
    }
    if (EliminationBotSound)
    {
        UGameplayStatics::SpawnSoundAtLocation(this, EliminationBotSound, SpawnPoint);
    }
}

void ABlasterCharacter::ZeroHUDAmmo()
{
    if (const auto PlayerController = Cast<ABlasterPlayerController>(Controller))
    {
        // Zero the Ammo on HUD
        PlayerController->SetHUDWeaponAmmo(0);
        ;
    }
}

void ABlasterCharacter::MulticastEliminate_Implementation()
{
    bEliminated = true;
    PlayEliminationMontage();
    StartDissolve();
    DisableCharacterMovement();
    DisableCollision();
    SpawnEliminationEffect();
    ZeroHUDAmmo();
}

void ABlasterCharacter::UpdateHUDHealth() const
{
    if (const auto PlayerController = Cast<ABlasterPlayerController>(Controller))
    {
        // Initialize Health on HUD
        PlayerController->SetHUDHealth(Health, MaxHealth);
    }
}

void ABlasterCharacter::Destroyed()
{
    Super::Destroyed();

    // We need to explicitly destroy component because we dynamically create component
    // and doing it in destroyed() as it is propagated to all clients which is what we want
    // as MulticastEliminate is propagated to all clients and that is where we create the component
    if (EliminationBotComponent)
    {
        EliminationBotComponent->DestroyComponent();
    }
}

void ABlasterCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (const auto PlayerController = Cast<APlayerController>(Controller); IsValid(PlayerController))
    {
        if (const auto Subsystem =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
            IsValid(Subsystem))
        {
            Subsystem->AddMappingContext(InputMapping.LoadSynchronous(), 0);
        }
    }

    if (const auto PlayerController = Cast<ABlasterPlayerController>(Controller))
    {
        PlayerController->ResetHUD();
    }
    if (HasAuthority())
    {
        // Where we have authority we make sure we react to damage
        OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::OnTakeDamage);
    }
}

void ABlasterCharacter::OnTakeDamage([[maybe_unused]] AActor* DamagedActor,
                                     const float Damage,
                                     [[maybe_unused]] const UDamageType* DamageType,
                                     [[maybe_unused]] AController* InstigatorController,
                                     [[maybe_unused]] AActor* DamageCauser)
{
    Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
    UpdateHUDHealth();
    PlayHitReactMontage();

    if (0.f == Health)
    {
        if (const auto GameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
        {
            const auto BlasterPlayerController = Cast<ABlasterPlayerController>(Controller);
            const auto AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
            GameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
        }
    }
}

void ABlasterCharacter::MoveInputActionTriggered(const FInputActionValue& Value)
{
    if (IsValid(Controller))
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FRotationMatrix RotationMatrix(YawRotation);
        // get forward vector
        const FVector ForwardDirection = RotationMatrix.GetUnitAxis(EAxis::X);

        // get right vector
        const FVector RightDirection = RotationMatrix.GetUnitAxis(EAxis::Y);

        // add movement
        const FVector2D MovementVector = Value.Get<FVector2D>();
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ABlasterCharacter::LookInputActionTriggered(const FInputActionValue& Value)
{
    if (IsValid(Controller))
    {
        // add yaw and pitch input to controller
        const FVector2D LookAxisVector = Value.Get<FVector2D>();
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ABlasterCharacter::EquipInputActionTriggered()
{
    if (HasAuthority())
    {
        if (IsValid(Combat))
        {
            Combat->EquipWeapon(OverlappingWeapon);
        }
    }
    else
    {
        // We are on the client so call the server
        ServerEquip();
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::ReloadInputActionTriggered()
{
    if (IsValid(Combat))
    {
        Combat->Reload();
    }
}

void ABlasterCharacter::OnCrouchInputActionStarted()
{
    Crouch();
}

void ABlasterCharacter::OnCrouchInputActionCompleted()
{
    UnCrouch();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::OnAimInputActionStarted()
{
    if (IsValid(Combat))
    {
        Combat->SetAiming(true);
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::OnAimInputActionCompleted()
{
    if (IsValid(Combat))
    {
        Combat->SetAiming(false);
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::OnFireInputActionStarted()
{
    if (IsValid(Combat))
    {
        Combat->SetFireButtonPressed(true);
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::OnFireInputActionCompleted()
{
    if (IsValid(Combat))
    {
        Combat->SetFireButtonPressed(false);
    }
}

void ABlasterCharacter::Jump()
{
    // Normally if you attempt to jump while crouched it is a no-op but when
    // we press "Jump" while crouching we will instead stand up.
    if (bIsCrouched)
    {
        UnCrouch();
    }
    else
    {
        Super::Jump();
    }
}

void ABlasterCharacter::PlayReloadMontage() const
{
    if (IsValid(ReloadMontage) && Combat && Combat->EquippedWeapon)
    {
        check(GetMesh());
        if (const auto AnimInstance = GetMesh()->GetAnimInstance(); IsValid(AnimInstance))
        {
            AnimInstance->Montage_Play(ReloadMontage);

            const EWeaponType WeaponType = Combat->EquippedWeapon->GetWeaponType();
            FName SectionName;
            if (EWeaponType::AssaultRifle == WeaponType)
            {
                SectionName = FName("Rifle");
            }
            AnimInstance->Montage_JumpToSection(SectionName);
        }
    }
}

void ABlasterCharacter::PlayEliminationMontage() const
{
    if (IsValid(EliminationMontage))
    {
        check(GetMesh());
        if (const auto AnimInstance = GetMesh()->GetAnimInstance(); IsValid(AnimInstance))
        {
            AnimInstance->Montage_Play(EliminationMontage);
            const FName SectionName("Default");
            AnimInstance->Montage_JumpToSection(SectionName);
        }
    }
}

void ABlasterCharacter::CalculateAimOffsetPitch()
{
    // Simply grab the pitch from where the character is aiming
    AimOffsetPitch = GetBaseAimRotation().Pitch;
    if (AimOffsetPitch > 180.f && !IsLocallyControlled())
    {
        // If BaseAimRotation is replicated across the network it is compressed into the
        // range 0-360 while if it is locally derived it can be outside that range and below zero
        // The AnimInstance uses a range -90 to 90 and thus a value of 270 (which is equivalent to -90)
        // will produce different values unless we perform this fixup below
        AimOffsetPitch -= 360.f;

        // This is the code from the course ... can't understand why the above is not used
        // const FVector2D InRange(270.f, 360.f);
        // const FVector2D OutRange(-90.f, 0.f);
        // AimOffsetPitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AimOffsetPitch);
    }
}

void ABlasterCharacter::CalculateAimOffset([[maybe_unused]] const float DeltaTime)
{
    if (!IsValid(Combat) || !IsValid(Combat->EquippedWeapon))
    {
        // If we have no weapon then we do not need to calculate aiming
        return;
    }
    const float Speed = CalculateSpeed();
    // ReSharper disable once CppTooWideScopeInitStatement
    const bool bInAir = GetCharacterMovement()->IsFalling();

    const FRotator BaseAimRotation = GetBaseAimRotation();

    CalculateAimOffsetPitch();

    // To calculate the yaw we need more complexity as when
    // we are moving it is the direction in which we are moving
    // otherwise when we are stopped it is where our controller is facing
    if (0.f == Speed && !bInAir)
    {
        // Standing still, not jumping

        bRotateRootBone = true;

        const FRotator CurrentAimRotation = FRotator(0.f, BaseAimRotation.Yaw, 0.f);
        const FRotator DeltaAimRotation =
            UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, AimOffsetBaseAimRotation);

        AimOffsetYaw = DeltaAimRotation.Yaw;
        if (ETurningInPlace::TIP_NotTurning == TurningInPlace)
        {
            AimOffsetYawInterp = AimOffsetYaw;
        }

        // We start using controller yaw for aiming and not character rotation
        bUseControllerRotationYaw = true;

        CalculateTurnInPlace(DeltaTime);
    }
    else
    {
        // Moving or jumping

        bRotateRootBone = false;

        AimOffsetBaseAimRotation = FRotator(0.f, BaseAimRotation.Yaw, 0.f);
        AimOffsetYaw = 0;

        // We revert to using the controller yaw for character rotation
        bUseControllerRotationYaw = true;
        TurningInPlace = ETurningInPlace::TIP_NotTurning;
    }
}

void ABlasterCharacter::SimProxiesTurn()
{
    // This is horrible. We fake turn in place animations for simulated proxies so that they look like
    // they are facing in the right direction without sliding feet. This is not what the autonomous proxy
    // does or what the server does and is an approximation so bullet shots that hit a simulated proxy may
    // not hit on server or vice versa. I am not sure this is how you would ever build a game - also concerning
    // is that the gun is never facing the actual target except for locally controlled actor ... which is also
    // horrible for a shooter platform.
    // I don't know why this is this way but just following tutorial in case future lessons depend upon it.
    // I assume the trainer just was not understand how to do this properly so we end up with this.

    check(Combat);
    if (Combat->EquippedWeapon)
    {
        bRotateRootBone = false;
        const float Speed = CalculateSpeed();
        if (Speed > 0.f)
        {
            TurningInPlace = ETurningInPlace::TIP_NotTurning;
        }
        else
        {
            const FRotator ProxyRotation = GetActorRotation();

            // Get the difference between last frames rotation and current frames rotation
            // ReSharper disable once CppTooWideScopeInitStatement
            const float ProxyYaw =
                UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

            // If we cross a threshold then change to turning in place animations
            if (ProxyYaw > ProxyTurnThreshold)
            {
                TurningInPlace = ETurningInPlace::TIP_TurningRight;
            }
            else if (ProxyYaw < -ProxyTurnThreshold)
            {
                TurningInPlace = ETurningInPlace::TIP_TurningLeft;
            }
            else
            {
                TurningInPlace = ETurningInPlace::TIP_NotTurning;
            }
            ProxyRotationLastFrame = ProxyRotation;
        }
    }
}

void ABlasterCharacter::CalculateTurnInPlace(const float DeltaTime)
{
    if (AimOffsetYaw > 90)
    {
        TurningInPlace = ETurningInPlace::TIP_TurningRight;
    }
    else if (AimOffsetYaw < -90)
    {
        TurningInPlace = ETurningInPlace::TIP_TurningLeft;
    }

    if (ETurningInPlace::TIP_NotTurning != TurningInPlace)
    {
        // Interp down to 0 over time
        AimOffsetYawInterp = FMath::FInterpTo(AimOffsetYawInterp, 0.f, DeltaTime, 4.f);
        AimOffsetYaw = AimOffsetYawInterp;
        if (FMath::Abs(AimOffsetYaw) < 15.f)
        {
            TurningInPlace = ETurningInPlace::TIP_NotTurning;
            AimOffsetBaseAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
        }
    }
}

void ABlasterCharacter::ServerEquip_Implementation()
{
    if (IsValid(Combat))
    {
        Combat->EquipWeapon(OverlappingWeapon);
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::OnRep_Health()
{
    UpdateHUDHealth();
    PlayHitReactMontage();
}

void ABlasterCharacter::RespawnTimerFinished()
{
    if (const auto GameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
    {
        GameMode->RequestRespawn(this, Controller);
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::UpdateDissolveMaterial(const float DissolveAmount)
{
    if (DynamicDissolveMaterialInstance)
    {
        DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("DissolveAmount"), DissolveAmount);
    }
}

void ABlasterCharacter::StartDissolve()
{
    if (DissolveCurve)
    {
        if (DissolveMaterialInstance)
        {
            // We do not need to worry about deallocating the DynamicDissolveMaterialInstance as it happens
            // just as the character is dying and just before the character id scheduled for destruction and the
            // PlayerController respawned attached to another Character so it will naturally be deleted
            DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
            DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("DissolveAmount"), 0.55f);
            DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("EmissiveFactor"), 200.f);

            GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
        }
        DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
        DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
        DissolveTimeline->Play();
    }
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* OldOverlappingWeapon) const
{
    if (IsValid(OldOverlappingWeapon))
    {
        // This hide is invoked on clients that receive OverlappingWeapon via replication
        OldOverlappingWeapon->ShowPickupWidget(false);
    }
    ShowPickupWidgetOnOverlappingWeapon(true);
}

void ABlasterCharacter::ShowPickupWidgetOnOverlappingWeapon(const bool bShowWidget) const
{
    if (IsValid(OverlappingWeapon))
    {
        OverlappingWeapon->ShowPickupWidget(bShowWidget);
    }
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
    if (IsLocallyControlled())
    {
        // This hide is invoked on listen server for locally controlled actors
        ShowPickupWidgetOnOverlappingWeapon(false);
    }
    OverlappingWeapon = Weapon;
    if (IsLocallyControlled())
    {
        ShowPickupWidgetOnOverlappingWeapon(true);
    }
}

AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{
    return IsWeaponEquipped() ? Combat->EquippedWeapon : nullptr;
}

bool ABlasterCharacter::IsWeaponEquipped() const
{
    return IsValid(Combat) && IsValid(Combat->EquippedWeapon);
}

FVector ABlasterCharacter::GetHitTarget() const
{
    return Combat ? Combat->HitTarget : FVector();
}

bool ABlasterCharacter::IsAiming() const
{
    return IsValid(Combat) && Combat->bAiming;
}

void ABlasterCharacter::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
    {
        // We get here if we are locally controlled and on the server node or locally controlled and on the client
        // Not sure why this is not equivalent to just IsLocallyControlled() ...
        CalculateAimOffset(DeltaTime);
    }
    else
    {
        TimeSinceLastMovementReplication += DeltaTime;
        if (TimeSinceLastMovementReplication > 0.25f)
        {
            // If it has been "long" enough since the last network tick hen fake it to try and avoid jitter
            OnRep_ReplicatedMovement();
        }
        CalculateAimOffsetPitch();
    }
    HideCharacterIfCameraClose();
}

void ABlasterCharacter::SafeBindAction(UEnhancedInputComponent* const Input,
                                       const TCHAR* Label,
                                       const TObjectPtr<UInputAction> InputAction,
                                       const ETriggerEvent TriggerEvent,
                                       void (ThisClass::*Func)(const FInputActionValue&))
{
    if (UNLIKELY(nullptr == InputAction))
    {
        UE_LOG(LogLoad, Error, TEXT("SetupPlayerInputComponent - InputAction property %s is not specified"), Label);
    }
    else
    {
        Input->BindAction(InputAction, TriggerEvent, this, Func);
    }
}

void ABlasterCharacter::HideCharacterIfCameraClose() const
{
    if (IsLocallyControlled())
    {
        const double BoomLength = (GetFollowCamera()->GetComponentLocation() - GetActorLocation()).Size();
        const bool bHideCharacter = BoomLength < CameraThreshold;

        // A better approach would be to have a camera fade threshold where the character
        // fades as it approaches camera. But this means changing materials or extracting a master material
        // with a camera depth fade node ... which was beyond this course
        GetMesh()->SetVisibility(!bHideCharacter);
        if (ensure(Combat) && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
        {
            Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = bHideCharacter;
        }
    }
}

float ABlasterCharacter::CalculateSpeed() const
{
    auto Velocity = GetVelocity();
    // We are only looking for lateral Velocity so we zero out height
    Velocity.Z = 0.f;
    return Velocity.Size();
}

void ABlasterCharacter::SafeBindAction(UEnhancedInputComponent* const Input,
                                       const TCHAR* Label,
                                       const TObjectPtr<UInputAction> InputAction,
                                       const ETriggerEvent TriggerEvent,
                                       void (ThisClass::*Func)())
{
    if (UNLIKELY(nullptr == InputAction))
    {
        UE_LOG(LogLoad, Error, TEXT("SetupPlayerInputComponent - InputAction property %s is not specified"), Label);
    }
    else
    {
        Input->BindAction(InputAction, TriggerEvent, this, Func);
    }
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Set up action bindings
    if (const auto Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jumping
        SafeBindAction(Input, TEXT("JumpAction"), JumpAction, ETriggerEvent::Started, &ABlasterCharacter::Jump);
        SafeBindAction(Input, TEXT("JumpAction"), JumpAction, ETriggerEvent::Completed, &ACharacter::StopJumping);

        // Moving
        SafeBindAction(Input,
                       TEXT("MoveAction"),
                       MoveAction,
                       ETriggerEvent::Triggered,
                       &ABlasterCharacter::MoveInputActionTriggered);

        // Looking action
        SafeBindAction(Input,
                       TEXT("LookAction"),
                       LookAction,
                       ETriggerEvent::Triggered,
                       &ABlasterCharacter::LookInputActionTriggered);

        // Perform equipping action
        SafeBindAction(Input,
                       TEXT("EquipAction"),
                       EquipAction,
                       ETriggerEvent::Triggered,
                       &ABlasterCharacter::EquipInputActionTriggered);

        SafeBindAction(Input,
                       TEXT("ReloadAction"),
                       ReloadAction,
                       ETriggerEvent::Triggered,
                       &ABlasterCharacter::ReloadInputActionTriggered);

        // Bind crouching actions
        SafeBindAction(Input,
                       TEXT("CrouchAction"),
                       CrouchAction,
                       ETriggerEvent::Started,
                       &ABlasterCharacter::OnCrouchInputActionStarted);
        SafeBindAction(Input,
                       TEXT("CrouchAction"),
                       CrouchAction,
                       ETriggerEvent::Completed,
                       &ABlasterCharacter::OnCrouchInputActionCompleted);

        // Bind Aiming actions
        SafeBindAction(Input,
                       TEXT("AimAction"),
                       AimAction,
                       ETriggerEvent::Started,
                       &ABlasterCharacter::OnAimInputActionStarted);
        SafeBindAction(Input,
                       TEXT("AimAction"),
                       AimAction,
                       ETriggerEvent::Completed,
                       &ABlasterCharacter::OnAimInputActionCompleted);

        // Bind Fire actions
        SafeBindAction(Input,
                       TEXT("FireAction"),
                       FireAction,
                       ETriggerEvent::Started,
                       &ABlasterCharacter::OnFireInputActionStarted);
        SafeBindAction(Input,
                       TEXT("FireAction"),
                       FireAction,
                       ETriggerEvent::Completed,
                       &ABlasterCharacter::OnFireInputActionCompleted);
    }
}
