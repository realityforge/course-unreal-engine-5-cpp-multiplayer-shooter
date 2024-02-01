#include "PlayerController/BlasterPlayerController.h"
#include "BlasterLogging.h"
#include "Character/BlasterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameMode/BlasterGameMode.h"
#include "HUD/Announcement.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/BlasterPlayerState.h"

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

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

void ABlasterPlayerController::HandleMatchHasStarted()
{
    if (const auto HUD = GetBlasterHUD())
    {
        HUD->AddCharacterOverlay();
        if (const auto& Announcement = HUD->GetAnnouncement())
        {
            // Once we actually start the match then hide the announcement HUD
            // We don't destroy it as we will use it at the end of a match as well
            Announcement->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void ABlasterPlayerController::HandleMatchInCooldown()
{
    check(IsLocalController());
    if (const auto& HUD = GetBlasterHUD())
    {
        HUD->GetCharacterOverlay()->RemoveFromParent();
        if (const auto& Announcement = HUD->GetAnnouncement())
        {
            Announcement->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void ABlasterPlayerController::UpdateHUDOnMatchStateChange()
{
    check(IsLocalController());
    if (MatchState::InProgress == MatchState)
    {
        HandleMatchHasStarted();
    }
    else if (MatchState::Cooldown == MatchState)
    {
        HandleMatchInCooldown();
    }
}

void ABlasterPlayerController::OnRep_MatchState()
{
    UpdateHUDOnMatchStateChange();
    LastTimeRemaining = 0.f;
}

void ABlasterPlayerController::InitHUDIfRequired()
{
    if (bInitializeCharacterOverlay && !CharacterOverlay && IsLocalController())
    {
        if (auto Overlay = GetCharacterOverlay())
        {
            CharacterOverlay = Overlay;
            SetHUDHealth(HUDHealth, HUDMaxHealth);
            SetHUDScore(HUDScore);
            SetHUDDefeats(HUDDefeats);
        }
    }
}

void ABlasterPlayerController::CacheBlasterHUD()
{
    check(IsLocalController());
    if (const auto HUD = GetHUD())
    {
        BlasterHUD = Cast<ABlasterHUD>(HUD);
        if (!BlasterHUD)
        {
            BL_ULOG_ERROR("HUD set to object named %s that can not be cast to ABlasterHUD", *HUD->GetName());
        }
    }
    else
    {
        BL_ULOG_ERROR("HUD not yet created, can not cache BlasterHUD");
    }
}

void ABlasterPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (IsLocalController())
    {
        CacheBlasterHUD();
    }

    // Retrieve match state from the server
    ServerCheckMatchState();
}

ABlasterHUD* ABlasterPlayerController::GetBlasterHUD()
{
    // This method should NOT be invoked unless the controller is local
    check(IsLocalController());

    if (UNLIKELY(!BlasterHUD))
    {
        // This happens when the actor ticks before BeginPlay which seems to happen on clients?
        CacheBlasterHUD();
    }
    return BlasterHUD;
}

UCharacterOverlay* ABlasterPlayerController::GetCharacterOverlay()
{
    check(IsLocalController());
    const auto HUD = GetBlasterHUD();
    return HUD ? HUD->GetCharacterOverlay() : nullptr;
}

void ABlasterPlayerController::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (IsLocalController())
    {
        UpdateHUDCountDown();
    }
    CheckTimeSync(DeltaSeconds);
    InitHUDIfRequired();
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
    check(IsLocalController());
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto& Overlay = GetCharacterOverlay();

    if (Overlay && Overlay->GetHealthBar() && Overlay->GetHealthText())
    {
        const float Percent = Health / MaxHealth;
        Overlay->GetHealthBar()->SetPercent(Percent);
        const auto& Text = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
        Overlay->GetHealthText()->SetText(FText::FromString(Text));
    }
    else
    {
        bInitializeCharacterOverlay = true;
        HUDHealth = Health;
        HUDMaxHealth = MaxHealth;
    }
}

void ABlasterPlayerController::SetHUDScore(const float Score)
{
    check(IsLocalController());
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto& Overlay = GetCharacterOverlay();
    if (Overlay && Overlay->GetScoreAmount())
    {
        const auto& Text = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
        Overlay->GetScoreAmount()->SetText(FText::FromString(Text));
    }
    else
    {
        bInitializeCharacterOverlay = true;
        HUDScore = Score;
    }
}

void ABlasterPlayerController::SetHUDDefeats(const int32 Defeats)
{
    check(IsLocalController());
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto& Overlay = GetCharacterOverlay();
    if (Overlay && Overlay->GetDefeatsAmount())
    {
        const auto& Text = FString::Printf(TEXT("%d"), Defeats);
        Overlay->GetDefeatsAmount()->SetText(FText::FromString(Text));
    }
    else
    {
        bInitializeCharacterOverlay = true;
        HUDDefeats = Defeats;
    }
}

void ABlasterPlayerController::SetHUDWeaponAmmo(const int32 Ammo)
{
    check(IsLocalController());
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
    check(IsLocalController());
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto& Overlay = GetCharacterOverlay();
    if (Overlay && Overlay->GetCarriedAmmoAmount())
    {
        const auto& Text = FString::Printf(TEXT("%d"), CarriedAmmo);
        Overlay->GetCarriedAmmoAmount()->SetText(FText::FromString(Text));
    }
}

void ABlasterPlayerController::SetHUDMatchCountDown(const int32 TimeRemaining)
{
    check(IsLocalController());
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto& Overlay = GetCharacterOverlay();
    if (Overlay && Overlay->GetCountDown())
    {
        const auto CountDown = Overlay->GetCountDown();
        if (TimeRemaining < 0.f)
        {
            CountDown->SetText(FText());
        }
        else
        {
            const int32 Minutes = TimeRemaining / 60;
            const int32 Seconds = TimeRemaining % 60;
            const auto& Text = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
            CountDown->SetText(FText::FromString(Text));
        }
    }
}

void ABlasterPlayerController::SetHUDAnnouncementCountDown(const int32 TimeRemaining)
{
    check(IsLocalController());
    // ReSharper disable once CppTooWideScopeInitStatement
    const auto HUD = GetBlasterHUD();
    if (HUD && HUD->GetAnnouncement())
    {
        const auto WarmupTime = HUD->GetAnnouncement()->GetWarmupTime();
        if (TimeRemaining < 0.f)
        {
            WarmupTime->SetText(FText());
        }
        else
        {
            const int32 Minutes = TimeRemaining / 60;
            const int32 Seconds = TimeRemaining % 60;
            const auto& Text = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
            WarmupTime->SetText(FText::FromString(Text));
        }
    }
}

void ABlasterPlayerController::UpdateHUDCountDown()
{
    const double LevelTime = GetWorld()->GetTimeSeconds() - LevelStartedAt;
    if (MatchState::WaitingToStart == MatchState)
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        const int32 TimeRemaining = FMath::FloorToInt32(WarmupDuration - LevelTime);
        if (LastTimeRemaining != TimeRemaining)
        {
            // Only update the UI when the text will change
            SetHUDAnnouncementCountDown(TimeRemaining);
            LastTimeRemaining = TimeRemaining;
        }
    }
    else if (MatchState::InProgress == MatchState)
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        const int32 TimeRemaining = FMath::FloorToInt32(WarmupDuration + MatchDuration - LevelTime);
        if (LastTimeRemaining != TimeRemaining)
        {
            // Only update the UI when the text will change
            SetHUDMatchCountDown(TimeRemaining);
            LastTimeRemaining = TimeRemaining;
        }
    }
}

void ABlasterPlayerController::ResetHUDIfLocalController()
{
    check(IsLocalController());
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
    if (IsLocalController())
    {
        // This makes sure we reset the hud on Respawn
        ResetHUDIfLocalController();
    }

    // Bit of an ugly hack because we delay match start which means
    // that we do not have controller when character BeginPlay is called
    if (const auto BlasterCharacter = Cast<ABlasterCharacter>(GetPawn()))
    {
        BlasterCharacter->RegisterPlayerInputMapping(this);
    }
}

void ABlasterPlayerController::OnMatchStateSet(const FName& State)
{
    MatchState = State;

    if (IsLocalController())
    {
        UpdateHUDOnMatchStateChange();
    }
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(const FName& InMatchState,
                                                                const float InWarmupDuration,
                                                                const float InMatchDuration,
                                                                const float InLevelStartedAt)
{
    WarmupDuration = InWarmupDuration;
    MatchDuration = InMatchDuration;
    LevelStartedAt = InLevelStartedAt;
    MatchState = InMatchState;
    if (IsLocalController())
    {
        UpdateHUDOnMatchStateChange();
        if (BlasterHUD && MatchState::WaitingToStart == MatchState)
        {
            BlasterHUD->AddAnnouncement();
        }
    }
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
    const auto GameMode = UGameplayStatics::GetGameMode(this);
    if (const auto BlasterGameMode = Cast<ABlasterGameMode>(GameMode))
    {
        // TODO: Unclear why we cache these here on the server?
        WarmupDuration = BlasterGameMode->GetWarmupDuration();
        MatchDuration = BlasterGameMode->GetMatchDuration();
        LevelStartedAt = BlasterGameMode->GetLevelStartedAt();
        MatchState = BlasterGameMode->GetMatchState();
        ClientJoinMidGame(MatchState, WarmupDuration, MatchDuration, LevelStartedAt);
    }
    else
    {
        BL_ULOG_ERROR("GameMode %s is not compatible with ABlasterGameMode", *GameMode->GetName());
    }
}
