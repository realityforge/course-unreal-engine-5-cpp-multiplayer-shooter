#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HUD/CharacterOverlay.h"
#include "BlasterPlayerController.generated.h"

class ABlasterHUD;

/**
 *
 */
UCLASS(Abstract)
class BLASTER_API ABlasterPlayerController : public APlayerController
{
    GENERATED_BODY()

    UPROPERTY(Transient)
    TObjectPtr<ABlasterHUD> BlasterHUD;

protected:
    virtual void BeginPlay() override;
    UCharacterOverlay* GetCharacterOverlay();

public:
    void SetHUDHealth(float Health, float MaxHealth);
    void SetHUDScore(float Score);
    void SetHUDDefeats(int32 Defeats);
    void ResetHUD();
    virtual void OnPossess(APawn* InPawn) override;
};
