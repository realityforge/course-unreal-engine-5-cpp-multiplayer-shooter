// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode/BlasterGameMode.h"
#include "Character/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* Character,
                                        ABlasterPlayerController* Controller,
                                        ABlasterPlayerController* Attacker)
{
    if (IsValid(Character))
    {
        Character->Eliminate();
    }
}
