#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HUD/CharacterOverlay.h"
#include "BlasterPlayerController.generated.h"

class ABlasterHUD;

UCLASS(Abstract)
class BLASTER_API ABlasterPlayerController : public APlayerController
{
    GENERATED_BODY()

    /** The duration (in seconds) that the match will stay in the Warmup phase. */
    UPROPERTY(Transient, VisibleInstanceOnly)
    float WarmupDuration{ 0.f };

    /** The duration (in seconds) of the match. */
    UPROPERTY(Transient, VisibleInstanceOnly)
    float MatchDuration{ 0.f };

    /** The time at which the level started. */
    UPROPERTY(Transient, VisibleInstanceOnly)
    float LevelStartedAt{ 0.f };

    /** The TimeRemaining when the HUD was last updated */
    int32 LastTimeRemaining{ 0 };

    //---------------------------------------------------------------------------
    // Deriving Match Time From Server
    //---------------------------------------------------------------------------

    /**
     * Request that the server send the server time to client.
     * The clients time is passed when request made so we can calculate round trip time.
     *
     * @param TimeOfClientRequest The time at which the client made this request. The time is in "client world" time.
     */
    UFUNCTION(Server, Reliable)
    void ServerRequestServerTime(float TimeOfClientRequest);

    /**
     * Called by the server after receiving ServerRequestServerTime to tell the client the server time.
     *
     * @param TimeOfClientRequest The time at which the client made the initial request. The time is in "client world"
     * time.
     * @param TimeServerReceivedClientRequest The time at which the server received and responded to the request.  The
     * time is in "server world" time.
     */
    UFUNCTION(Client, Reliable)
    void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

    /**
     * The difference between the time on the server and the time on the client.
     */
    float ClientServerTimeDelta{ 0.f };

    /**
     * The time since the last time we attempted to synchronize time.
     */
    float TimeSinceLastTimeSync{ 0.f };

    /** The frequency which we attempt to synchronize server time and client time. */
    UPROPERTY(EditDefaultsOnly, Category = "Time")
    float TimeSyncFrequency{ 5.f };

    float GetServerTime() const;
    void CheckTimeSync(float DeltaTime);
    void HandleMatchHasStarted();
    void UpdateHUDOnMatchStateChange();

    //---------------------------------------------------------------------------
    // MatchState (why oh why is this per-PlayerController? Why not replicate it on GameMode?)
    //---------------------------------------------------------------------------

    UPROPERTY(ReplicatedUsing = OnRep_MatchState)
    FName MatchState;

    UFUNCTION()
    void OnRep_MatchState();

    //---------------------------------------------------------------------------
    // HUD and CharacterOverlay
    //---------------------------------------------------------------------------

    UPROPERTY(Transient)
    TObjectPtr<ABlasterHUD> BlasterHUD{ nullptr };

    UCharacterOverlay* GetCharacterOverlay();
    ABlasterHUD* GetBlasterHUD();

    /** A cached copy of CharacterOverlay. */
    UPROPERTY(Transient)
    TObjectPtr<UCharacterOverlay> CharacterOverlay{ nullptr };

    /** Set when we have some data to put on the HUD and thus should initialize HUD if not initialized. */
    bool bInitializeCharacterOverlay{ false };

    // The following are all cached values HUD which we will use when we
    // initialize the HUD. Thereafter they are not used.

    float HUDHealth{ 0.f };
    float HUDMaxHealth{ 0.f };
    float HUDScore{ 0.f };
    int32 HUDDefeats{ 0 };

    void InitHUDIfRequired();

    /** Used to cache the BlasterHUD reference. */
    void CacheBlasterHUD();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaSeconds) override;
    virtual void ReceivedPlayer() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void SetHUDHealth(float Health, float MaxHealth);
    void SetHUDScore(float Score);
    void SetHUDDefeats(int32 Defeats);
    void SetHUDWeaponAmmo(int32 Ammo);
    void SetHUDCarriedAmmo(int32 CarriedAmmo);
    void SetHUDMatchCountDown(int32 MatchTimeRemaining);
    void SetHUDAnnouncementCountdown(int32 PreMatchTimeRemaining);

    void UpdateHUDCountDown();

    void ResetHUDIfLocalController();
    virtual void OnPossess(APawn* InPawn) override;

    void OnMatchStateSet(const FName& State);

    void HandleMatchInCooldown();

    /** Called by client to update the local match state. */
    UFUNCTION(Server, Reliable)
    void ServerCheckMatchState();

    /** Callback that is called from ServerCheckMatchState to update client with match details. */
    UFUNCTION(Client, Reliable)
    void
    ClientJoinMidGame(const FName& InMatchState, float InWarmupDuration, float InMatchDuration, float InLevelStartedAt);
};
