#include "GameMode/BlasterGameMode.h"
#include "Character/BlasterCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/BlasterPlayerController.h"
#include "PlayerState/BlasterPlayerState.h"

ABlasterGameMode::ABlasterGameMode()
{
    // The GameMode will not automatically transition from WaitingToStart to InProgress
    // when we set bDelayedStart to true so we have to manually call StartMatch
    bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{
    Super::BeginPlay();

    LevelStartedAt = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::Tick(const float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (MatchState::WaitingToStart == MatchState)
    {
        // Start the match after we have spent WarmupDuration waiting
        if (WarmupDuration - GetWorld()->GetTimeSeconds() + LevelStartedAt <= 0.f)
        {
            StartMatch();
        }
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* Character,
                                        const ABlasterPlayerController* Controller,
                                        const ABlasterPlayerController* Attacker)
{
    const auto AttackerState = Attacker ? Cast<ABlasterPlayerState>(Attacker->PlayerState) : nullptr;
    const auto VictimState = Controller ? Cast<ABlasterPlayerState>(Controller->PlayerState) : nullptr;

    if (AttackerState && AttackerState != VictimState)
    {
        AttackerState->AddToScore(1.f);
    }
    if (VictimState)
    {
        VictimState->AddToDefeats(1);
    }
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
