// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
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

    /**
     * @brief Called when session creation completes.
     *
     * NOTE: This needs to be a UFUNCTION as it is bound to a dynamic delegate!
     *
     * @param bWasSuccessful True if session was successfully created, false otherwise.
     */
    UFUNCTION()
    void OnCreateSessionComplete(bool bWasSuccessful);

private:
    UPROPERTY(meta = (BindWidget))
    UButton* HostButton;

    UPROPERTY(meta = (BindWidget))
    UButton* JoinButton;

    UFUNCTION()
    void OnHostButtonClicked();

    UFUNCTION()
    void OnJoinButtonClicked();

    UPROPERTY()
    UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

    void MenuTearDown();

    int32 NumPublicConnections{ 4 };
    FString MatchType{ TEXT("FreeForAll") };
};
