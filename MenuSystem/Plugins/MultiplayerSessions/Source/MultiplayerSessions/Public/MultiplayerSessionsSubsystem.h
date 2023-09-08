// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplayerSessionsSubsystem.generated.h"

/**
 *
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMultiplayerSessionsSubsystem();

    /**
     * @brief Create session in the Online platform.
     * @param NumPublicConnections The maximum number of players in the game
     * @param MatchType The type of game that is being created
     */
    void CreateSession(int32 NumPublicConnections, const FString& MatchType);

    /**
     * @brief
     * @param MaxSearchResults the maximum number of search results.
     */
    void FindSessions(int32 MaxSearchResults);

    /**
     * @brief Join the session as specified by search result.
     * @param SessionResult the session to join.
     */
    void JoinSession(const FOnlineSessionSearchResult& SessionResult);

    /**
     * @brief Destroy the active session if any.
     */
    void DestroySession();

    /**
     * @brief Start the session.
     */
    void StartSession();

protected:
    // ---------------------------------------------------------------------------------
    // Callbacks bound to delegates and ultimately invoked from OnlineSession Interface
    // ---------------------------------------------------------------------------------

    /**
     * @brief Called when a session create request has completed
     * @param SessionName the name of the session this callback is for
     * @param bWasSuccessful true if the async action completed without error,
     * false if there was an error
     */
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

    /**
     * @brief Called when the search for an online session has completed
     * @param bWasSuccessful true if the async action completed without error,
     * false if there was an error
     */
    void OnFindSessionsComplete(bool bWasSuccessful);

    /**
     * @brief Called when the joining process for an online session has completed
     * @param SessionName the name of the session this callback is for
     * @param Result the result of the attempt to join the session.
     */
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

    /**
     * @brief Called when destroying an online session has completed
     * @param SessionName the name of the session this callback is for
     * @param bWasSuccessful true if the async action completed without error, false if there was an error
     */
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

    /**
     * @brief Called when the online session has transitioned to the started state
     * @param SessionName the name of the session the that has transitioned to started
     * @param bWasSuccessful true if the async action completed without error, false if there was an error
     */
    void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
    IOnlineSessionPtr OnlineSessionInterface;

    TSharedPtr<FOnlineSessionSettings> LastSessionSettings;

    // ---------------------------------------------------------------------------------
    // The following delegates are added to the OnlineSessionInterface and bound to local callbacks.
    // ---------------------------------------------------------------------------------

    FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
    FDelegateHandle FindSessionsCompleteDelegateHandle;
    FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
    FDelegateHandle JoinSessionCompleteDelegateHandle;
    FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
    FDelegateHandle DestroySessionCompleteDelegateHandle;
    FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
    FDelegateHandle StartSessionCompleteDelegateHandle;
};
