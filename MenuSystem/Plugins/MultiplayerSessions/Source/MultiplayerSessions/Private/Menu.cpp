// Fill out your copyright notice in the Description page of Project Settings.

#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"

UMenu::UMenu(const FObjectInitializer& ObjectInitializer)
    : UUserWidget(ObjectInitializer), HostButton(nullptr), JoinButton(nullptr), MultiplayerSessionsSubsystem(nullptr)
{
}

void UMenu::MenuSetup(int32 InNumPublicConnections, FString InMatchType)
{
    NumPublicConnections = InNumPublicConnections;
    MatchType = InMatchType;

    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    SetIsFocusable(true);

    if (const auto World = GetWorld())
    {
        if (const auto PlayerController = World->GetFirstPlayerController())
        {
            // Data structure used to setup an input mode that allows only the UI to respond to user input
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
            // Data structure used to setup an input mode that allows only the Game to respond to user input
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
        return true;
    }
    else
    {
        return false;
    }
}

void UMenu::NativeDestruct()
{
    // Invoked when the underlying ui widget disappears which means we can tear down menu
    // infrastructure and re-enable user input to the game

    MenuTearDown();

    Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("UMenu::OnCreateSessionComplete ")));
    }
    if (bWasSuccessful)
    {
        if (const auto World = GetWorld())
        {
            World->ServerTravel(FString("Lobby?listen"));
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

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("UMenu::OnFindSessionsComplete ")));
    }
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("UMenu::OnJoinSessionComplete ")));
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
        MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
    }
}

void UMenu::OnJoinButtonClicked()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Join button clicked!")));
    }
}
