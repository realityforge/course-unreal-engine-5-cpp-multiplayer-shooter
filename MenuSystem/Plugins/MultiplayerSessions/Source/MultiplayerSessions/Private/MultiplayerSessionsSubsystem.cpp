// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
    : CreateSessionCompleteDelegate(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete))
    , FindSessionsCompleteDelegate(
          FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnFindSessionsComplete))
    , JoinSessionCompleteDelegate(
          FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionComplete))
    , DestroySessionCompleteDelegate(
          FOnDestroySessionCompleteDelegate::CreateUObject(this,
                                                           &UMultiplayerSessionsSubsystem::OnDestroySessionComplete))
    , StartSessionCompleteDelegate(
          FOnStartSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnStartSessionComplete))
{
    const auto Subsystem = IOnlineSubsystem::Get();
    OnlineSessionInterface = Subsystem ? Subsystem->GetSessionInterface() : nullptr;
}

void UMultiplayerSessionsSubsystem::CompleteSessionCreate(const bool bWasSuccessful)
{
    // Remove the delegate  on session interface
    OnlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

    // Notify listeners that session creation completed either successfully or as a failure
    MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, const FString& MatchType)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("CreateSession")));
    }
    if (ensure(OnlineSessionInterface))
    {
        if (OnlineSessionInterface->GetNamedSession(NAME_GameSession))
        {
            // DestroySession is async ... presumably we should wait until destroy
            // completes Before moving onto next phase ... we should do something if
            // the session was not successfully deleted which we do not ... weird ...
            if (!OnlineSessionInterface->DestroySession(NAME_GameSession, DestroySessionCompleteDelegate))
            {
                UE_LOG(LogTemp, Error, TEXT("UMultiplayerSessionsSubsystem: Failed to DestroySession.\n"));
            }
        }
        CreateSessionCompleteDelegateHandle =
            OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

        LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
        const auto OnlineSubsystem = Online::GetSubsystem(GetWorld());

        // Identify OnlineSubsystemNull which we can run on a lan if needed.
        const bool isNullOnlineSubsystem = "NULL" == OnlineSubsystem->GetSubsystemName();
        LastSessionSettings->bIsLANMatch = isNullOnlineSubsystem ? true : false;
        LastSessionSettings->NumPublicConnections = NumPublicConnections;
        LastSessionSettings->bAllowJoinInProgress = true;
        LastSessionSettings->bAllowJoinViaPresence = true;
        LastSessionSettings->bShouldAdvertise = true;
        LastSessionSettings->bUsesPresence = true;

        // This sets a property on our session so that later when we look up sessions we can look for this key
        // to make sure we get the desired type of session
        LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

        const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
        UE_LOG(LogTemp, Error, TEXT("UMultiplayerSessionsSubsystem: Calling CreateSession.\n"));
        if (!OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),
                                                   NAME_GameSession,
                                                   *LastSessionSettings))
        {
            CompleteSessionCreate(false);
        }
    }
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("FindSessions")));
    }
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("JoinSession")));
    }
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("DestroySession")));
    }
}

void UMultiplayerSessionsSubsystem::StartSession()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("StartSession")));
    }
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("OnCreateSessionComplete")));
    }
    CompleteSessionCreate(true);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("OnFindSessionsComplete")));
    }
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("OnJoinSessionComplete")));
    }
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("OnDestroySessionComplete")));
    }
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("OnStartSessionComplete")));
    }
}
