#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

UCLASS()
class ALobbyGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    ALobbyGameMode();
    virtual void PostLogin(APlayerController* NewPlayer) override;
};
