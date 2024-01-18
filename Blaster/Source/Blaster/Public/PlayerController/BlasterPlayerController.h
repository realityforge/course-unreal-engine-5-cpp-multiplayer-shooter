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

    /** The duration of the match in seconds. */
    UPROPERTY(EditAnywhere)
    int32 MatchDuration{ 120 };

    /** The MatchTimeRemaining when the HUD was last updated */
    int32 LastMatchTimeRemaining{ 0 };

protected:
    virtual void BeginPlay() override;
    UCharacterOverlay* GetCharacterOverlay();

public:
    virtual void Tick(float DeltaSeconds) override;

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
