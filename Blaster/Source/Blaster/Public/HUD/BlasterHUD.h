// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
    GENERATED_BODY()

    UPROPERTY()
    UTexture2D* CrosshairsCenter{ nullptr };
    UPROPERTY()
    UTexture2D* CrosshairsLeft{ nullptr };
    UPROPERTY()
    UTexture2D* CrosshairsRight{ nullptr };
    UPROPERTY()
    UTexture2D* CrosshairsTop{ nullptr };
    UPROPERTY()
    UTexture2D* CrosshairsBottom{ nullptr };
};

/**
 *
 */
UCLASS(Abstract)
class BLASTER_API ABlasterHUD : public AHUD
{
    GENERATED_BODY()

    FHUDPackage HUDPackage;

public:
    virtual void DrawHUD() override;

    FORCEINLINE void SetHUDPackage(const FHUDPackage& InHUDPackage) { HUDPackage = InHUDPackage; }
};
