// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode/BlasterGameMode.h"
#include "Character/BlasterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
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

AActor* ABlasterGameMode::SelectPlayerStart() const
{
    // The algorithm is to randomly select a player start location
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
    const int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
    return PlayerStarts[Selection];
}

void ABlasterGameMode::RequestRespawn(ACharacter* Character, AController* Controller)
{
    UE_LOG(LogTemp,
           Verbose,
           TEXT("ABlasterGameMode::RequestRespawn %s"),
           (Character ? *Character->GetName() : TEXT("?")));
    if (Character)
    {
        // Resetting disassociates the controller from character
        // among other things
        Character->Reset();
        Character->Destroy();
    }
    if (Controller)
    {
        RestartPlayerAtPlayerStart(Controller, SelectPlayerStart());
    }
}
