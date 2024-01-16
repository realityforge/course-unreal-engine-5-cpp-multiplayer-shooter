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

protected:
    virtual void BeginPlay() override;
    UCharacterOverlay* GetCharacterOverlay();

public:
    void SetHUDHealth(float Health, float MaxHealth);
    void SetHUDScore(float Score);
    void SetHUDDefeats(int32 Defeats);
    void SetHUDWeaponAmmo(int32 Ammo);
    void ResetHUD();
    virtual void OnPossess(APawn* InPawn) override;
};
