#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterPlayerController;
class ABlasterCharacter;

UCLASS()
class BLASTER_API ABlasterGameMode final : public AGameMode
{
    GENERATED_BODY()

    /** The duration that the match will stay in the Warmup phase. */
    UPROPERTY(EditDefaultsOnly)
    float WarmupDuration{ 10.f };

    /** The time at which the level started. */
    UPROPERTY(Transient)
    float LevelStartedAt{ 0.f };

    /**
     * Select a player start location.
     *
     * @return the start location.
     */
    AActor* SelectPlayerStart() const;

protected:
    virtual void BeginPlay() override;
    virtual void OnMatchStateSet() override;

public:
    ABlasterGameMode();

    virtual void Tick(float DeltaTime) override;

    /**
     * Invoked when a player is Eliminated/killed.
     *
     * @param Character the Character representing the Player eliminated/killed.
     * @param Controller the PlayerController representing the Player eliminated/killed.
     * @param Attacker The PlayerController that eliminated/killed the Player.
     */
    void PlayerEliminated(ABlasterCharacter* Character,
                          const ABlasterPlayerController* Controller,
                          const ABlasterPlayerController* Attacker);

    /**
     * Request that the player be respawned.
     *
     * @param Character The Character that represented the player.
     * @param Controller The Player Controller.
     */
    void RequestRespawn(ACharacter* Character, AController* Controller);
};
