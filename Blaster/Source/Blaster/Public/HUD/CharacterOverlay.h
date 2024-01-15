// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "CharacterOverlay.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS(Abstract)
class BLASTER_API UCharacterOverlay : public UUserWidget
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, meta = (BindWidget, AllowPrivateAccess))
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(VisibleAnywhere, meta = (BindWidget, AllowPrivateAccess))
    TObjectPtr<UTextBlock> HealthText;

    UPROPERTY(VisibleAnywhere, meta = (BindWidget, AllowPrivateAccess))
    TObjectPtr<UTextBlock> ScoreAmount;

    UPROPERTY(VisibleAnywhere, meta = (BindWidget, AllowPrivateAccess))
    TObjectPtr<UTextBlock> DefeatsAmount;

public:
    FORCEINLINE UProgressBar* GetHealthBar() const { return HealthBar; }
    FORCEINLINE UTextBlock* GetHealthText() const { return HealthText; }
    FORCEINLINE UTextBlock* GetScoreAmount() const { return ScoreAmount; }
    FORCEINLINE UTextBlock* GetDefeatsAmount() const { return DefeatsAmount; }
};
