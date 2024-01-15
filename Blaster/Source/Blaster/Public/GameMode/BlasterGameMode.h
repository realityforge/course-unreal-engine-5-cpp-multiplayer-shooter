#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterPlayerController;
class ABlasterCharacter;

UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
    GENERATED_BODY()

    /**
     * Select a player start location.
     *
     * @return the start location.
     */
    AActor* SelectPlayerStart() const;

public:
    /**
     * Invoked when a player is Eliminated/killed.
     *
     * @param Character the Character representing the Player eliminated/killed.
     * @param Controller the PlayerController representing the Player eliminated/killed.
     * @param Attacker The PlayerController that eliminated/killed the Player.
     */
    virtual void PlayerEliminated(ABlasterCharacter* Character,
                                  ABlasterPlayerController* Controller,
                                  ABlasterPlayerController* Attacker);

    /**
     * Request that the player be respawned.
     *
     * @param Character The Character that represented the player.
     * @param Controller The Player Controller.
     */
    virtual void RequestRespawn(ACharacter* Character, AController* Controller);
};
