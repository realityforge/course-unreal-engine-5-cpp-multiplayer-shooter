// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD/BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
    Super::DrawHUD();

    if (GEngine)
    {
        FVector2D ViewportSize;
        GEngine->GameViewport->GetViewportSize(ViewportSize);
        const FVector2D ViewportCenter{ ViewportSize.X / 2.f, ViewportSize.Y / 2.f };

        const float Spread = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

        // Crosshairs* textures can be null if no weapon is equipped
        if (HUDPackage.CrosshairsTop)
        {
            DrawCrossHair(HUDPackage.CrosshairsTop, ViewportCenter, FVector2D{ 0.f, -Spread });
        }
        if (HUDPackage.CrosshairsBottom)
        {
            DrawCrossHair(HUDPackage.CrosshairsBottom, ViewportCenter, FVector2D{ 0.f, Spread });
        }
        if (HUDPackage.CrosshairsCenter)
        {
            DrawCrossHair(HUDPackage.CrosshairsCenter, ViewportCenter, FVector2D{ 0.f, 0.f });
        }
        if (HUDPackage.CrosshairsLeft)
        {
            DrawCrossHair(HUDPackage.CrosshairsLeft, ViewportCenter, FVector2D{ -Spread, 0.f });
        }
        if (HUDPackage.CrosshairsRight)
        {
            DrawCrossHair(HUDPackage.CrosshairsRight, ViewportCenter, FVector2D{ Spread, 0.f });
        }
    }
}

void ABlasterHUD::DrawCrossHair(UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D& Spread)
{
    const float TextureWidth = Texture->GetSizeX();
    const float TextureHeight = Texture->GetSizeY();
    // Offset origin of draw based on dimensions of icon and spread
    const float OriginX = ViewportCenter.X - (TextureWidth / 2.f) + Spread.X;
    const float SpreadY = ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y;
    const FLinearColor& TintColor = FLinearColor::White;
    DrawTexture(Texture, OriginX, SpreadY, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, TintColor);
}
