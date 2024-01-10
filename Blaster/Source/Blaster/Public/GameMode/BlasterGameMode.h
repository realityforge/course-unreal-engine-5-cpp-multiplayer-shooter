// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterPlayerController;
class ABlasterCharacter;

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    /**
     * Invoked when a player is ELiminated/killed.
     *
     * @param Character the Character representing the Player eliminated.
     * @param Controller the PlayerController representing the Player eliminated.
     * @param Attacker The PlayerController that eliminated the Player.
     */
    virtual void PlayerEliminated(ABlasterCharacter* Character,
                                  ABlasterPlayerController* Controller,
                                  ABlasterPlayerController* Attacker);
};