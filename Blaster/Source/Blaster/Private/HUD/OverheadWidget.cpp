// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD/OverheadWidget.h"
#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(const FString& TextToDisplay) const
{
    if (DisplayText)
    {
        DisplayText->SetText(FText::FromString(TextToDisplay));
    }
}

void UOverheadWidget::ShowPlayerNetRole(const APawn* InPawn) const
{
    FString Role;
    switch (InPawn->GetLocalRole())
    {
        case ROLE_Authority:
            Role = FString("Authority");
            break;
        case ROLE_AutonomousProxy:
            Role = FString("AutonomousProxy");
            break;
        case ROLE_SimulatedProxy:
            Role = FString("SimulatedProxy");
            break;
        case ROLE_None:
            Role = FString("None");
            break;
        default:
            Role = FString("Unknown");
    }
    SetDisplayText(FString::Printf(TEXT("Local Role: %s"), *Role));
}

void UOverheadWidget::NativeDestruct()
{
    RemoveFromParent();
    Super::NativeDestruct();
}
