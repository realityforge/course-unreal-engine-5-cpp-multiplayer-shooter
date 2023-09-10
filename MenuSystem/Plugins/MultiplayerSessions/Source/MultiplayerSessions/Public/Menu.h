// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

class UButton;
class UMultiplayerSessionsSubsystem;

/**
 * A base class for widgets interacting with the MultiplayerSessions system.
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
    GENERATED_BODY()

protected:
    UMenu(const FObjectInitializer& ObjectInitializer);

    virtual void NativeDestruct() override;

public:
    UFUNCTION(BlueprintCallable)
    void MenuSetup(int32 InNumPublicConnections = 4, FString InMatchType = TEXT("FreeForAll"));

    virtual bool Initialize() override;

protected:
    // Callbacks bound to delegates from MultiplayerOnCreateSessionComplete
    // NOTE: UFUNCTION added where passed to a dynamic delegate!

    /**
     * @brief Called when session creation completes.
     * @param bWasSuccessful True if session was successfully created, false otherwise.
     */
    UFUNCTION()
    void OnCreateSession(bool bWasSuccessful);

    /**
     * @brief invoked when search  operation completes.
     * @param SearchResults The results returned from find query
     * @param bWasSuccessful True if operation was successful, false otherwise.
     */
    void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);

    /**
     * @brief Called when Join session attempt completes.
     * @param Result Result of join attempt.
     */
    void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

    /**
     * @brief Called when session destroy completes.
     * @param bWasSuccessful True if session was successfully created, false otherwise.
     */
    UFUNCTION()
    void OnDestroySession(bool bWasSuccessful);

    /**
     * @brief Called when start session completes.
     * @param bWasSuccessful True if operation was successful, false otherwise.
     */
    UFUNCTION()
    void OnStartSession(bool bWasSuccessful);

private:
    UPROPERTY(meta = (BindWidget))
    UButton* HostButton{ nullptr };

    UPROPERTY(meta = (BindWidget))
    UButton* JoinButton{ nullptr };

    UFUNCTION()
    void OnHostButtonClicked();

    UFUNCTION()
    void OnJoinButtonClicked();

    UPROPERTY()
    UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem{ nullptr };

    void MenuTearDown();

    int32 NumPublicConnections{ 4 };
    FString MatchType{ TEXT("FreeForAll") };
};
