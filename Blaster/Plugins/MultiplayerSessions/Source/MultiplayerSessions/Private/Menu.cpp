// Fill out your copyright notice in the Description page of Project Settings.

#include "Menu.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

UMenu::UMenu(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer) {}

void UMenu::MenuSetup(const int32 InNumPublicConnections, const FString& InMatchType, const FString& InPathToLobby)
{
    NumPublicConnections = InNumPublicConnections;
    MatchType = InMatchType;
    PathToLobby = InPathToLobby;

    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    SetIsFocusable(true);

    if (const auto World = GetWorld())
    {
        if (const auto PlayerController = World->GetFirstPlayerController())
        {
            // Data structure used to setup an input mode that allows only the UI to
            // respond to user input
            FInputModeUIOnly InputModeData;

            InputModeData.SetWidgetToFocus(TakeWidget());

            // Don't lock mouse cursor to the viewport so mouse can move off screen
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

            PlayerController->SetInputMode(InputModeData);

            // Make the mouse visible so user can see what they are doing
            PlayerController->SetShowMouseCursor(true);
        }
    }
    if (const auto GameInstance = GetGameInstance())
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    }

    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UMenu::OnCreateSession);
        MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &UMenu::OnFindSessions);
        MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &UMenu::OnJoinSession);
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UMenu::OnDestroySession);
        MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &UMenu::OnStartSession);
    }
}

void UMenu::MenuTearDown()
{
    // The "inverse" of MenuSetup above

    RemoveFromParent();

    if (const auto World = GetWorld())
    {
        if (const auto PlayerController = World->GetFirstPlayerController())
        {
            // Data structure used to setup an input mode that allows only the Game to
            // respond to user input
            const FInputModeGameOnly InputModeData;

            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(false);
        }
    }
}

bool UMenu::Initialize()
{
    if (Super::Initialize())
    {
        if (HostButton)
        {
            HostButton->OnClicked.AddDynamic(this, &UMenu::OnHostButtonClicked);
        }
        if (JoinButton)
        {
            JoinButton->OnClicked.AddDynamic(this, &UMenu::OnJoinButtonClicked);
        }
        if (QuitButton)
        {
            QuitButton->OnClicked.AddDynamic(this, &UMenu::OnQuitButtonClicked);
        }
        return true;
    }
    else
    {
        return false;
    }
}

void UMenu::NativeDestruct()
{
    // Invoked when the underlying ui widget disappears which means we can tear
    // down menu infrastructure and re-enable user input to the game

    MenuTearDown();

    Super::NativeDestruct();
}

void UMenu::OnCreateSession(const bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("UMenu::OnCreateSessionComplete ")));
    }
    HostButton->SetIsEnabled(true);
    if (bWasSuccessful)
    {
        if (const auto World = GetWorld())
        {
            const FString URL = FString::Printf(TEXT("%s?listen"), *PathToLobby);
            World->ServerTravel(URL);
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("Failed to create session")));
        }
    }
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, const bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1,
                                         15.f,
                                         FColor::Yellow,
                                         FString::Printf(TEXT("UMenu::OnFindSessionsComplete - "
                                                              "bWasSuccessful=%d SearchResults.Len=%d"),
                                                         bWasSuccessful,
                                                         SearchResults.Num()));
    }
    if (MultiplayerSessionsSubsystem)
    {
        for (const auto Result : SearchResults)
        {
            // Look for a result that matches our desired MatchType
            if (FString ResultMatchType;
                Result.Session.SessionSettings.Get(FName("MatchType"), ResultMatchType) && ResultMatchType == MatchType)
            {
                MultiplayerSessionsSubsystem->JoinSession(Result);
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(
                        -1,
                        15.f,
                        FColor::Yellow,
                        FString(TEXT("UMenu::OnFindSessionsComplete - Found matching session")));
                }
                return;
            }
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1,
                                             15.f,
                                             FColor::Yellow,
                                             FString(TEXT("UMenu::OnFindSessionsComplete - Failed to get subsystem")));
        }
    }
    // This will capture all the error scenarios, success will be handled in
    // OnJoinSession
    JoinButton->SetIsEnabled(true);
}

void UMenu::OnJoinSession(const EOnJoinSessionCompleteResult::Type Result)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("UMenu::OnJoinSessionComplete ")));
    }
    if (const auto Subsystem = Online::GetSubsystem(GetWorld()))
    {
        if (const auto SessionInterface = Subsystem->GetSessionInterface(); SessionInterface.IsValid())
        {
            if (FString Address; SessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
            {
                if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
                {
                    PlayerController->ClientTravel(Address, TRAVEL_Absolute);
                    JoinButton->SetIsEnabled(true);
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(
                            -1,
                            15.f,
                            FColor::Yellow,
                            FString(TEXT("UMenu::OnJoinSessionComplete - Client Travel Complete")));
                    }
                }
                else
                {
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1,
                                                         15.f,
                                                         FColor::Yellow,
                                                         FString(TEXT("UMenu::OnJoinSessionComplete - No local player "
                                                                      "controller")));
                    }
                }
            }
            else
            {
                // TODO: Error here?
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1,
                                                     15.f,
                                                     FColor::Yellow,
                                                     FString(TEXT("UMenu::OnJoinSessionComplete - No get Address")));
                }
            }
        }
        else
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1,
                    15.f,
                    FColor::Yellow,
                    FString(TEXT("UMenu::OnJoinSessionComplete - Invalid Session INterface")));
            }
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1,
                                             15.f,
                                             FColor::Yellow,
                                             FString(TEXT("UMenu::OnJoinSessionComplete - Failed to get subsystem")));
        }
    }
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("UMenu::OnDestroySessionComplete ")));
    }
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("UMenu::OnStartSessionComplete ")));
    }
}

void UMenu::OnHostButtonClicked()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Host button clicked!")));
    }
    if (MultiplayerSessionsSubsystem)
    {
        HostButton->SetIsEnabled(false);
        MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
    }
}

void UMenu::OnJoinButtonClicked()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Join button clicked!")));
    }
    if (MultiplayerSessionsSubsystem)
    {
        JoinButton->SetIsEnabled(false);
        // We set a very high session count as we are using the DevId for game and
        // lots of other devs will be adding sessions
        MultiplayerSessionsSubsystem->FindSessions(10000);
    }
    else
    {
        // TODO: Really should deal gracefully with this error
    }
}

void UMenu::OnQuitButtonClicked()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Quit button clicked!")));
    }
    if (const auto World = GetWorld())
    {
        if (const auto PlayerController = World->GetFirstPlayerController())
        {

            UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, false);
        }
    }
}
