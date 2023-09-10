// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameModeBase::OutputNumberOfPlayersInGame(const int32 NumberOfPlayers)
{
    if (GameState)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(1,
                                             60.f,
                                             FColor::Yellow,
                                             FString::Printf(TEXT("Players in Game: %d"), NumberOfPlayers));
        }
    }
}

void ALobbyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
    OutputNumberOfPlayersInGame(NumberOfPlayers);

    if (GEngine)
    {
        if (const auto PlayerState = NewPlayer->GetPlayerState<APlayerState>(); ensure(PlayerState))
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Cyan,
                FString::Printf(TEXT("%s has joined the game"), *PlayerState->GetPlayerName()));
        }
    }
}

void ALobbyGameModeBase::Logout(AController* Exiting)
{
    const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
    // Player count has yet to be updated so just pass -1
    OutputNumberOfPlayersInGame(NumberOfPlayers - 1);

    if (const auto PlayerState = Exiting->GetPlayerState<APlayerState>(); ensure(PlayerState))
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            15.f,
            FColor::Cyan,
            FString::Printf(TEXT("%s has exited the game"), *PlayerState->GetPlayerName()));
    }
    Super::Logout(Exiting);
}
