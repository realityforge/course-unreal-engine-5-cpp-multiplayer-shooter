// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/BlasterCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"

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
}

void ABlasterCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (const auto PlayerController = Cast<APlayerController>(Controller))
    {
        if (const auto Subsystem =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(InputMapping.LoadSynchronous(), 0);
        }
    }
}

void ABlasterCharacter::Move(const FInputActionValue& Value)
{
    if (Controller)
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

void ABlasterCharacter::Look(const FInputActionValue& Value)
{
    if (Controller)
    {
        // add yaw and pitch input to controller
        const FVector2D LookAxisVector = Value.Get<FVector2D>();
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ABlasterCharacter::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Set up action bindings
    if (const auto Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jumping
        Input->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
        Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

        // Moving
        Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Move);

        // Looking
        Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Look);
    }
}
