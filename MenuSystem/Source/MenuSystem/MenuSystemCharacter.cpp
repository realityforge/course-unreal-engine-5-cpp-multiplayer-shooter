// Copyright Epic Games, Inc. All Rights Reserved.

#include "MenuSystemCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

//////////////////////////////////////////////////////////////////////////
// AMenuSystemCharacter

AMenuSystemCharacter::AMenuSystemCharacter()
    : CreateSessionCompleteDelegate(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &AMenuSystemCharacter::OnCreateSessionComplete))
    , FindSessionsCompleteDelegate(
          FOnFindSessionsCompleteDelegate::CreateUObject(this, &AMenuSystemCharacter::OnFindSessionsComplete))
    , JoinSessionCompleteDelegate(
          FOnJoinSessionCompleteDelegate::CreateUObject(this, &AMenuSystemCharacter::OnJoinSessionComplete))
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;            // Character moves in the direction of input...
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

    // Note: For faster iteration times these variables, and many more, can be
    // tweaked in the Character Blueprint instead of recompiling to adjust them
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;       // The camera follows at this distance behind the character
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    // Attach the camera to the end of the boom and let the boom adjust to match
    // the controller orientation
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Note: The skeletal mesh and anim blueprint references on the Mesh component
    // (inherited from Character) are set in the derived blueprint asset named
    // ThirdPersonCharacter (to avoid direct content references in C++)

    if (const auto OnlineSubsystem = IOnlineSubsystem::Get())
    {
        OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Blue,
                FString::Printf(TEXT("Found subsystem %s"), *OnlineSubsystem->GetSubsystemName().ToString()));
        }
    }
}

void AMenuSystemCharacter::BeginPlay()
{
    // Call the base class
    Super::BeginPlay();

    // Add Input Mapping Context
    if (const auto PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void AMenuSystemCharacter::CreateGameSession()
{
    if (OnlineSessionInterface.IsValid())
    {
        // There can not be multiple sessions so if we try to create another then we
        // should destroy the current
        if (OnlineSessionInterface->GetNamedSession(NAME_GameSession))
        {
            // DestroySession is async ... presumably we should wait until destroy
            // completes Before moving onto next phase ... we should do something if
            // the session was not successfully deleted which we do not ... weird ...
            if (!OnlineSessionInterface->DestroySession(NAME_GameSession))
            {
                UE_LOG(LogTemp, Error, TEXT("AMenuSystemCharacter: Failed to DestroySession.\n"));
            }
        }

        OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

        const TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
        SessionSettings->bIsLANMatch = false;
        SessionSettings->NumPublicConnections = 4;
        SessionSettings->bAllowJoinInProgress = true;
        SessionSettings->bAllowJoinViaPresence = true;
        SessionSettings->bShouldAdvertise = true;
        SessionSettings->bUsesPresence = true;
        // This sets a property on our session so that later when we look up sessions we can look for this key
        // to make sure we get the "right" session
        SessionSettings->Set(FName("MatchType"),
                             FString("FreeForAll"),
                             EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

        // TODO: Maybe try this
        // SessionSettings->bUseLobbiesIfAvailable = true;

        const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
        UE_LOG(LogTemp, Error, TEXT("AMenuSystemCharacter: Calling CreateSession.\n"));
        OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),
                                              NAME_GameSession,
                                              *SessionSettings);
    }
}

void AMenuSystemCharacter::JoinGameSession()
{
    if (!OnlineSessionInterface.IsValid())
    {
        return;
    }

    // TODO: Unclear why we do not do this in the constructor ... rather than
    // potentially adding multiple
    OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

    // Find Game Sessions
    // TODO: Unclear why this is not setup in ctor?
    SessionSearch = MakeShareable(new FOnlineSessionSearch());

    // We set a very high session count as we are using the DevId for game and
    // lots of other devs will be adding sessions
    SessionSearch->MaxSearchResults = 10000;
    SessionSearch->bIsLanQuery = false;

    // Make sure any sessions we find are using presence
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}

void AMenuSystemCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Error, TEXT("AMenuSystemCharacter: OnCreateSessionComplete(%d).\n"), bWasSuccessful);
    if (bWasSuccessful)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1,
                                             15.f,
                                             FColor::Blue,
                                             FString::Printf(TEXT("Created Session %s"), *SessionName.ToString()));
        }
        UWorld* World = GetWorld();
        if (World)
        {
            World->ServerTravel(FString("/Game/ThirdPerson/Maps/Lobby?listen"));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Red,
                FString::Printf(TEXT("Failed to create Session %s"), *SessionName.ToString()));
        }
    }
}

void AMenuSystemCharacter::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (bWasSuccessful && OnlineSessionInterface.IsValid())
    {
        for (auto Result : SessionSearch->SearchResults)
        {
            FString SessionId = Result.GetSessionIdStr();
            FString OwningUserName = Result.Session.OwningUserName;
            FUniqueNetIdPtr OwningUserId = Result.Session.OwningUserId;
            // Extract the MatchType we care about
            FString MatchType = FString();
            Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1,
                    15.f,
                    FColor::Cyan,
                    FString::Printf(TEXT("Id: %s, User: %s"), *SessionId, *OwningUserName));
            }
            if (FString("FreeForAll") == MatchType)
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1,
                                                     15.f,
                                                     FColor::Cyan,
                                                     FString::Printf(TEXT("Joining MatchType: %s"), *MatchType));
                }
                // Unclear why this is not setup once when session established?
                OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

                const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
                OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result);
            }
        }
    }
}

void AMenuSystemCharacter::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (OnlineSessionInterface.IsValid())
    {
        FString Address;
        if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1,
                                                 15.f,
                                                 FColor::Yellow,
                                                 FString::Printf(TEXT("ConnectInfo: %s"), *Address));
            }
            if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
            {
                PlayerController->ClientTravel(Address, TRAVEL_Absolute);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMenuSystemCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jumping
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

        // Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMenuSystemCharacter::Move);

        // Looking
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMenuSystemCharacter::Look);
    }
}

void AMenuSystemCharacter::Move(const FInputActionValue& Value)
{
    // input is a Vector2D
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // get forward vector
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        // get right vector
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // add movement
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AMenuSystemCharacter::Look(const FInputActionValue& Value)
{
    // input is a Vector2D
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // add yaw and pitch input to controller
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}
