#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

class ABlasterPlayerState;

UCLASS(Abstract)
class BLASTER_API ABlasterGameState : public AGameState
{
    GENERATED_BODY()

    UPROPERTY()
    float TopScore{ 0.f };

    UPROPERTY(Replicated)
    TArray<TObjectPtr<ABlasterPlayerState>> TopScoringPlayers;

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);

    FORCEINLINE TArray<TObjectPtr<ABlasterPlayerState>> GetTopScoringPlayers() const { return TopScoringPlayers; }
};
