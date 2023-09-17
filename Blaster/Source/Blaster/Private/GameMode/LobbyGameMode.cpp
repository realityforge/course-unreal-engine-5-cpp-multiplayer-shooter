#include "GameMode/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

ALobbyGameMode::ALobbyGameMode()
{
    bUseSeamlessTravel = true;
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num(); 2 == NumberOfPlayers)
    {
        if (const auto World = GetWorld())
        {
            World->ServerTravel(FString(TEXT("/Game/Maps/BasterMap?listen")));
        }
    }
}
