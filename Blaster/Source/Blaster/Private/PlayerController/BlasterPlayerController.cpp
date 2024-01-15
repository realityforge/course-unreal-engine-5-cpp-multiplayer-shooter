#include "PlayerController/BlasterPlayerController.h"
#include "Character/BlasterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay.h"
#include "PlayerState/BlasterPlayerState.h"

void ABlasterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

UCharacterOverlay* ABlasterPlayerController::GetCharacterOverlay()
{
    if (UNLIKELY(!BlasterHUD))
    {
        // What scenario is this actually required?
        BlasterHUD = Cast<ABlasterHUD>(GetHUD());
    }

    return BlasterHUD ? BlasterHUD->GetCharacterOverlay() : nullptr;
}

void ABlasterPlayerController::SetHUDHealth(const float Health, const float MaxHealth)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto& Overlay = GetCharacterOverlay();

    if (Overlay && Overlay->GetHealthBar() && Overlay->GetHealthText())
    {
        const float Percent = Health / MaxHealth;
        Overlay->GetHealthBar()->SetPercent(Percent);
        const auto& Text = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
        Overlay->GetHealthText()->SetText(FText::FromString(Text));
    }
}

void ABlasterPlayerController::SetHUDScore(const float Score)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto& Overlay = GetCharacterOverlay();
    if (Overlay && Overlay->GetScoreAmount())
    {
        const auto& Text = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
        Overlay->GetScoreAmount()->SetText(FText::FromString(Text));
    }
}

void ABlasterPlayerController::SetHUDDefeats(const int32 Defeats)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto& Overlay = GetCharacterOverlay();
    if (Overlay && Overlay->GetDefeatsAmount())
    {
        const auto& Text = FString::Printf(TEXT("%d"), Defeats);
        Overlay->GetDefeatsAmount()->SetText(FText::FromString(Text));
    }
}

void ABlasterPlayerController::ResetHUD()
{
    if (const auto BlasterCharacter = Cast<ABlasterCharacter>(GetPawn()))
    {
        SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
        if (const auto BlasterPlayerState = GetPlayerState<ABlasterPlayerState>())
        {
            SetHUDScore(BlasterPlayerState->GetScore());
            SetHUDDefeats(BlasterPlayerState->GetDefeats());
        }
    }
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    // This makes sure we reset the hud on Respawn
    ResetHUD();
}
