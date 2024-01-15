#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "CharacterOverlay.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS(Abstract)
class BLASTER_API UCharacterOverlay final : public UUserWidget
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, meta = (BindWidget, AllowPrivateAccess))
    TObjectPtr<UProgressBar> HealthBar{ nullptr };

    UPROPERTY(VisibleAnywhere, meta = (BindWidget, AllowPrivateAccess))
    TObjectPtr<UTextBlock> HealthText{ nullptr };

    UPROPERTY(VisibleAnywhere, meta = (BindWidget, AllowPrivateAccess))
    TObjectPtr<UTextBlock> ScoreAmount{ nullptr };

    UPROPERTY(VisibleAnywhere, meta = (BindWidget, AllowPrivateAccess))
    TObjectPtr<UTextBlock> DefeatsAmount{ nullptr };

public:
    FORCEINLINE UProgressBar* GetHealthBar() const { return HealthBar; }
    FORCEINLINE UTextBlock* GetHealthText() const { return HealthText; }
    FORCEINLINE UTextBlock* GetScoreAmount() const { return ScoreAmount; }
    FORCEINLINE UTextBlock* GetDefeatsAmount() const { return DefeatsAmount; }
};
