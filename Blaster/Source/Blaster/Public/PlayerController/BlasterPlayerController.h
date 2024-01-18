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

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaSeconds) override;
    virtual void ReceivedPlayer() override;

    void SetHUDHealth(float Health, float MaxHealth);
    void SetHUDScore(float Score);
    void SetHUDDefeats(int32 Defeats);
    void SetHUDWeaponAmmo(int32 Ammo);
    void SetHUDCarriedAmmo(int32 CarriedAmmo);
    void SetHUDCountDown(int32 MatchTimeRemaining);

    void UpdateHUDCountDown();

    void ResetHUD();
    virtual void OnPossess(APawn* InPawn) override;
};
