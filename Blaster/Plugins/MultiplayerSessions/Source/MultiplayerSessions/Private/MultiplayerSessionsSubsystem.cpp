// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionsSubsystem.h"
#include "Online/OnlineSessionNames.h"
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
}

bool UMultiplayerSessionsSubsystem::IsNullOnlineSubsystem() const
{
    const auto OnlineSubsystem = Online::GetSubsystem(GetWorld());
    return "NULL" == OnlineSubsystem->GetSubsystemName();
}

IOnlineSubsystem* UMultiplayerSessionsSubsystem::GetOnlineSubsystem() const
{
    return Online::GetSubsystem(GetWorld());
}

IOnlineSessionPtr UMultiplayerSessionsSubsystem::GetOnlineSessionInterface() const
{
    const auto Subsystem = GetOnlineSubsystem();
    return Subsystem ? Subsystem->GetSessionInterface() : nullptr;
}

// -----------------------------------------------------------------------------------------------------------
// Create Session
// -----------------------------------------------------------------------------------------------------------

void UMultiplayerSessionsSubsystem::CreateSession(const int32 NumPublicConnections, const FString& MatchType)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("CreateSession")));
    }
    if (const auto OnlineSessionInterface = GetOnlineSessionInterface(); ensure(OnlineSessionInterface))
    {
        if (OnlineSessionInterface->GetNamedSession(NAME_GameSession))
        {
            bCreateSessionOnDestroy = true;
            LastNumPublicConnections = NumPublicConnections;
            LastMatchType = MatchType;
            DestroySession();
        }
        else
        {
            CreateSessionCompleteDelegateHandle =
                OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

            LastSessionSettings = MakeShareable(new FOnlineSessionSettings());

            LastSessionSettings->bIsLANMatch = IsNullOnlineSubsystem() ? true : false;
            LastSessionSettings->NumPublicConnections = NumPublicConnections;
            LastSessionSettings->bAllowJoinInProgress = true;
            LastSessionSettings->bAllowJoinViaPresence = true;
            LastSessionSettings->bShouldAdvertise = true;
            LastSessionSettings->bUsesPresence = true;
            LastSessionSettings->BuildUniqueId = 1;

            // This sets a property on our session so that later when we look up
            // sessions we can look for this key to make sure we get the desired type
            // of session
            LastSessionSettings->Set(FName("MatchType"),
                                     MatchType,
                                     EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

            UE_LOG(LogTemp, Error, TEXT("UMultiplayerSessionsSubsystem: Calling CreateSession.\n"));
            if (!OnlineSessionInterface->CreateSession(
                    *GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId(),
                    NAME_GameSession,
                    *LastSessionSettings))
            {
                CompleteSessionCreate(false);
            }
        }
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

void UMultiplayerSessionsSubsystem::CompleteSessionCreate(const bool bWasSuccessful)
{
    if (const auto OnlineSessionInterface = GetOnlineSessionInterface(); OnlineSessionInterface.IsValid())
    {
        // Remove the delegate  on session interface
        OnlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }
    else
    {
        CreateSessionCompleteDelegateHandle.Reset();
    }

    // Notify listeners that session creation completed either successfully or as
    // a failure
    MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

// -----------------------------------------------------------------------------------------------------------
// Find Sessions
// -----------------------------------------------------------------------------------------------------------

void UMultiplayerSessionsSubsystem::FindSessions(const int32 MaxSearchResults)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("FindSessions")));
    }
    if (const auto OnlineSessionInterface = GetOnlineSessionInterface(); OnlineSessionInterface.IsValid())
    {
        FindSessionsCompleteDelegateHandle =
            OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

        LastSessionSearch = MakeShareable(new FOnlineSessionSearch());

        LastSessionSearch->MaxSearchResults = MaxSearchResults;
        LastSessionSearch->bIsLanQuery = IsNullOnlineSubsystem() ? true : false;

        // Make sure any sessions we find are using presence
        LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

        if (!OnlineSessionInterface->FindSessions(
                *GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId(),
                LastSessionSearch.ToSharedRef()))
        {
            // failed to find sessions so cleanup state
            const TArray<FOnlineSessionSearchResult> Results;
            CompleteFindSessions(Results, false);
        }
    }
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("OnFindSessionsComplete")));
    }

    CompleteFindSessions(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::CompleteFindSessions(const TArray<FOnlineSessionSearchResult>& Results,
                                                         const bool bWasSuccessful)
{
    if (const auto OnlineSessionInterface = GetOnlineSessionInterface(); OnlineSessionInterface.IsValid())
    {
        OnlineSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
    }
    else
    {
        FindSessionsCompleteDelegateHandle.Reset();
    }

    // A successful search that returns zero results is converted to a failure for
    // listeners
    MultiplayerOnFindSessionsComplete.Broadcast(Results, bWasSuccessful && Results.Num() > 0);
}

// -----------------------------------------------------------------------------------------------------------
// Join Session
// -----------------------------------------------------------------------------------------------------------

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("JoinSession")));
    }
    if (const auto OnlineSessionInterface = GetOnlineSessionInterface(); OnlineSessionInterface.IsValid())
    {
        JoinSessionCompleteDelegateHandle =
            OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

        if (!OnlineSessionInterface->JoinSession(
                *GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId(),
                NAME_GameSession,
                SessionResult))
        {
            CompleteJoinSession(EOnJoinSessionCompleteResult::UnknownError);
        }
    }
    else
    {
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
    }
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("OnJoinSessionComplete")));
    }
    CompleteJoinSession(Result);
}

void UMultiplayerSessionsSubsystem::CompleteJoinSession(const EOnJoinSessionCompleteResult::Type ResultType)
{
    if (const auto OnlineSessionInterface = GetOnlineSessionInterface(); OnlineSessionInterface.IsValid())
    {
        OnlineSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
    }
    else
    {
        JoinSessionCompleteDelegateHandle.Reset();
    }
    MultiplayerOnJoinSessionComplete.Broadcast(ResultType);
}

// -----------------------------------------------------------------------------------------------------------
// Destroy Session
// -----------------------------------------------------------------------------------------------------------

void UMultiplayerSessionsSubsystem::DestroySession()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("DestroySession")));
    }
    if (const auto OnlineSessionInterface = GetOnlineSessionInterface(); OnlineSessionInterface.IsValid())
    {
        DestroySessionCompleteDelegateHandle =
            OnlineSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
        if (!OnlineSessionInterface->DestroySession(NAME_GameSession))
        {
            DestroySessionComplete(false);
        }
    }
    else
    {
        DestroySessionComplete(false);
    }
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("OnDestroySessionComplete")));
    }
    DestroySessionComplete(bWasSuccessful);
    if (bCreateSessionOnDestroy)
    {
        if (bWasSuccessful)
        {
            const int32 NumPublicConnections = LastNumPublicConnections;
            const FString MatchType = LastMatchType;
            LastNumPublicConnections = 0;
            LastMatchType = "";
            bCreateSessionOnDestroy = false;
            CreateSession(NumPublicConnections, MatchType);
        }
        else
        {
            UE_LOG(LogTemp,
                   Error,
                   TEXT("UMultiplayerSessionsSubsystem Failed to Destroy session "
                        "when a create session was pening.\n"));
            // Unclear what to do here ... call session create failed?
        }
    }
}

void UMultiplayerSessionsSubsystem::DestroySessionComplete(bool bWasSuccessful)
{
    if (DestroySessionCompleteDelegateHandle.IsValid())
    {
        if (const auto OnlineSessionInterface = GetOnlineSessionInterface(); OnlineSessionInterface.IsValid())
        {
            OnlineSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
        }
        else
        {
            DestroySessionCompleteDelegateHandle.Reset();
        }
    }
    MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

// -----------------------------------------------------------------------------------------------------------
// Start Session
// -----------------------------------------------------------------------------------------------------------

void UMultiplayerSessionsSubsystem::StartSession()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("StartSession")));
    }
    if (const auto OnlineSessionInterface = GetOnlineSessionInterface(); OnlineSessionInterface.IsValid())
    {
        StartSessionCompleteDelegateHandle =
            OnlineSessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

        if (!OnlineSessionInterface->StartSession(NAME_GameSession))
        {
            CompleteStartSession(false);
        }
    }
    else
    {
        CompleteStartSession(false);
    }
}

void UMultiplayerSessionsSubsystem::CompleteStartSession(const bool bWasSuccessful)
{
    if (const auto OnlineSessionInterface = GetOnlineSessionInterface(); OnlineSessionInterface.IsValid())
    {
        OnlineSessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
    }
    else
    {
        StartSessionCompleteDelegateHandle.Reset();
    }
    MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Emerald, FString(TEXT("OnStartSessionComplete")));
    }
    CompleteStartSession(bWasSuccessful);
}
