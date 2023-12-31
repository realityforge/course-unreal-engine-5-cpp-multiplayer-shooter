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
        // Crosshairs* textures can be null if no weapon is equipped
        if (HUDPackage.CrosshairsTop)
        {
            DrawCrossHair(HUDPackage.CrosshairsTop, ViewportCenter);
        }
        if (HUDPackage.CrosshairsBottom)
        {
            DrawCrossHair(HUDPackage.CrosshairsBottom, ViewportCenter);
        }
        if (HUDPackage.CrosshairsCenter)
        {
            DrawCrossHair(HUDPackage.CrosshairsCenter, ViewportCenter);
        }
        if (HUDPackage.CrosshairsLeft)
        {
            DrawCrossHair(HUDPackage.CrosshairsLeft, ViewportCenter);
        }
        if (HUDPackage.CrosshairsRight)
        {
            DrawCrossHair(HUDPackage.CrosshairsRight, ViewportCenter);
        }
    }
}

void ABlasterHUD::DrawCrossHair(UTexture2D* Texture, const FVector2D& ViewportCenter)
{
    const float TextureWidth = Texture->GetSizeX();
    const float TextureHeight = Texture->GetSizeY();
    // Offset origin of draw based on dimensions of icon
    const FVector2D Origin{ ViewportCenter.X - (TextureWidth / 2.f), ViewportCenter.Y - (TextureHeight / 2.f) };
    const FLinearColor& TintColor = FLinearColor::White;
    DrawTexture(Texture, Origin.X, Origin.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, TintColor);
}
