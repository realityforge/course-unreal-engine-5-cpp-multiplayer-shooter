// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerController/BlasterPlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay.h"

void ABlasterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::SetHUDHealth(const float Health, const float MaxHealth)
{
    if (UNLIKELY(!BlasterHUD))
    {
        // What scenario is this actually required?
        BlasterHUD = Cast<ABlasterHUD>(GetHUD());
    }

    // ReSharper disable once CppTooWideScopeInitStatement
    const UCharacterOverlay* Overlay = BlasterHUD ? BlasterHUD->GetCharacterOverlay() : nullptr;

    if (Overlay && Overlay->GetHealthBar() && Overlay->GetHealthText())
    {
        const float Percent = Health / MaxHealth;
        Overlay->GetHealthBar()->SetPercent(Percent);
        const auto HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
        Overlay->GetHealthText()->SetText(FText::FromString(HealthText));
    }
}
