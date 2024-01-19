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

    UPROPERTY(Transient)
    TObjectPtr<ABlasterHUD> BlasterHUD{ nullptr };

    UCharacterOverlay* GetCharacterOverlay();
    ABlasterHUD* GetBlasterHUD();

    /** The duration of the match in seconds. */
    UPROPERTY(EditAnywhere)
    int32 MatchDuration{ 120 };

    /** The MatchTimeRemaining when the HUD was last updated */
    int32 LastMatchTimeRemaining{ 0 };

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
    void AddCharacterOverlayIfMatchStateInProgress();

    //---------------------------------------------------------------------------
    // MatchState (why oh why is this per-PlayerController? Why not replicate it on GameMode?)
    //---------------------------------------------------------------------------

    UPROPERTY(ReplicatedUsing = OnRep_MatchState)
    FName MatchState;

    UFUNCTION()
    void OnRep_MatchState();

    //---------------------------------------------------------------------------
    // CharacterOverlay
    //---------------------------------------------------------------------------

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
    void SetHUDCountDown(int32 MatchTimeRemaining);

    void UpdateHUDCountDown();

    void ResetHUD();
    virtual void OnPossess(APawn* InPawn) override;

    void OnMatchStateSet(const FName& State);
};
