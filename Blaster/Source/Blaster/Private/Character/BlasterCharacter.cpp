#include "Character/BlasterCharacter.h"
#include "BlasterComponents/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

ABlasterCharacter::ABlasterCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

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

    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;

    OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
    OverheadWidget->SetupAttachment(GetMesh());

    Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
    Combat->SetIsReplicated(true);

    // The CharacterMovement component already supports crouching - we just need to enable it.
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

    // Make sure when another character moves between our camera and our character, the camera boom
    // does not move in. We do this by making sure we do not block camera
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ABlasterCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    if (IsValid(Combat))
    {
        Combat->Character = this;
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

void ABlasterCharacter::OnCrouchInputActionStarted()
{
    Crouch();
}

void ABlasterCharacter::OnCrouchInputActionCompleted()
{
    UnCrouch();
}

void ABlasterCharacter::OnAimInputActionStarted()
{
    if (IsValid(Combat))
    {
        Combat->setAiming(true);
    }
}

void ABlasterCharacter::OnAimInputActionCompleted()
{
    if (IsValid(Combat))
    {
        Combat->setAiming(false);
    }
}

void ABlasterCharacter::ServerEquip_Implementation()
{
    if (IsValid(Combat))
    {
        Combat->EquipWeapon(OverlappingWeapon);
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

bool ABlasterCharacter::IsWeaponEquipped() const
{
    return IsValid(Combat) && IsValid(Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming() const
{
    return IsValid(Combat) && Combat->bAiming;
}

void ABlasterCharacter::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);
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
        SafeBindAction(Input, TEXT("JumpAction"), JumpAction, ETriggerEvent::Started, &ACharacter::Jump);
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
    }
}
