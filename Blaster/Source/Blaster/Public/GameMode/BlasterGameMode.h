#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterPlayerController;
class ABlasterCharacter;

// Reopen MatchState namespace to add our custom MatchState
namespace MatchState
{
    /** Match duration has been reached. Display winner and begin cooldown timer. */
    extern BLASTER_API const FName Cooldown;
} // namespace MatchState

UCLASS()
class BLASTER_API ABlasterGameMode final : public AGameMode
{
    GENERATED_BODY()

    /** The duration (in seconds) that the match will stay in the Warmup phase. */
    UPROPERTY(EditDefaultsOnly)
    float WarmupDuration{ 10.f };

    /** The duration (in seconds) of the match. */
    UPROPERTY(EditDefaultsOnly)
    float MatchDuration{ 120.f };

    /** The duration (in seconds) that the match will stay in the Cooldown phase. */
    UPROPERTY(EditDefaultsOnly)
    float CooldownDuration{ 10.f };

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

    FORCEINLINE float GetWarmupDuration() const { return WarmupDuration; }
    FORCEINLINE float GetMatchDuration() const { return MatchDuration; }
    FORCEINLINE float GetLevelStartedAt() const { return LevelStartedAt; }
};
