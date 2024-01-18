#include "PlayerController/BlasterPlayerController.h"
#include "Character/BlasterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "EnhancedInputSubsystems.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay.h"
#include "PlayerState/BlasterPlayerState.h"

void ABlasterPlayerController::ServerRequestServerTime_Implementation(const float TimeOfClientRequest)
{
    check(HasAuthority());
    ClientReportServerTime(TimeOfClientRequest, GetWorld()->GetTimeSeconds());
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(const float TimeOfClientRequest,
                                                                     const float TimeServerReceivedClientRequest)
{
    const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
    // Guestimate that the RoundTrip spent 50% of time on message being transported to server
    // and 50% of time being sent back to client which is wrong as most connections have asymmetrical
    // connection speeds. But it is accurate enough for our purposes.
    const float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
    ClientServerTimeDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlasterPlayerController::GetServerTime() const
{
    const double WorldTime = GetWorld()->GetTimeSeconds();
    return HasAuthority() ? WorldTime : WorldTime + ClientServerTimeDelta;
}

void ABlasterPlayerController::CheckTimeSync(const float DeltaTime)
{
    if (IsLocalController())
    {
        TimeSinceLastTimeSync += DeltaTime;
        if (TimeSinceLastTimeSync > TimeSyncFrequency)
        {
            ServerRequestServerTime(GetWorld()->GetTimeSeconds());
            TimeSinceLastTimeSync = 0.f;
        }
    }
}

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

void ABlasterPlayerController::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateHUDCountDown();
    CheckTimeSync(DeltaSeconds);
}

void ABlasterPlayerController::ReceivedPlayer()
{
    Super::ReceivedPlayer();
    // Sync with server clock as soon as possible
    if (IsLocalController())
    {
        ServerRequestServerTime(GetWorld()->GetTimeSeconds());
    }
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

void ABlasterPlayerController::SetHUDWeaponAmmo(const int32 Ammo)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto& Overlay = GetCharacterOverlay();
    if (Overlay && Overlay->GetAmmoAmount())
    {
        const auto& Text = FString::Printf(TEXT("%d"), Ammo);
        Overlay->GetAmmoAmount()->SetText(FText::FromString(Text));
    }
}

void ABlasterPlayerController::SetHUDCarriedAmmo(const int32 CarriedAmmo)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto& Overlay = GetCharacterOverlay();
    if (Overlay && Overlay->GetCarriedAmmoAmount())
    {
        const auto& Text = FString::Printf(TEXT("%d"), CarriedAmmo);
        Overlay->GetCarriedAmmoAmount()->SetText(FText::FromString(Text));
    }
}

void ABlasterPlayerController::SetHUDCountDown(const int32 MatchTimeRemaining)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto& Overlay = GetCharacterOverlay();
    if (Overlay && Overlay->GetCountDown())
    {
        const int32 Minutes = MatchTimeRemaining / 60;
        const int32 Seconds = MatchTimeRemaining % 60;
        const auto& Text = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
        Overlay->GetCountDown()->SetText(FText::FromString(Text));
    }
}

void ABlasterPlayerController::UpdateHUDCountDown()
{
    const double MatchStartTime = GetWorld()->GetTimeSeconds();
    // ReSharper disable once CppTooWideScopeInitStatement
    const int32 MatchTimeRemaining = FMath::FloorToInt32(MatchDuration - MatchStartTime);
    if (LastMatchTimeRemaining != MatchTimeRemaining)
    {
        // Only update the UI when the text will change
        SetHUDCountDown(MatchTimeRemaining);
        LastMatchTimeRemaining = MatchTimeRemaining;
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
            // Should we derive this from equipped weapon just in case we
            // respawn with a weapon equipped?
            SetHUDWeaponAmmo(0);
            // Also what happens if we respawn with ammo?
            SetHUDCarriedAmmo(0);
        }
    }
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    // This makes sure we reset the hud on Respawn
    ResetHUD();

    // Bit of an ugly hack because we delay match start which means
    // that we do not have controller when character BeginPlay is called
    if (const auto BlasterCharacter = Cast<ABlasterCharacter>(GetPawn()))
    {
        BlasterCharacter->RegisterPlayerInputMapping(this);
    }
}
