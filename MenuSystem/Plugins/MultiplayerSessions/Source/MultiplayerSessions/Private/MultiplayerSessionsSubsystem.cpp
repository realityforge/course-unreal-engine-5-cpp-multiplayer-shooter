// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

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

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, const FString& MatchType) {}
void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults) {}
void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult) {}
void UMultiplayerSessionsSubsystem::DestroySession() {}
void UMultiplayerSessionsSubsystem::StartSession() {}
void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful) {}
void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful) {}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful) {}
void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful) {}
