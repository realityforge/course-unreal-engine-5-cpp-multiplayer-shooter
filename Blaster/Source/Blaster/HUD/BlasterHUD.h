#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

class UAnnouncement;
class UCharacterOverlay;

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
    // The factor applied to CrosshairSpreadMax
    UPROPERTY()
    float CrosshairSpread{ 0.f };
    /** The color to draw crosshairs */
    UPROPERTY()
    FLinearColor CrosshairColor{ FLinearColor::White };
};

UCLASS(Abstract)
class BLASTER_API ABlasterHUD final : public AHUD
{
    GENERATED_BODY()

    FHUDPackage HUDPackage;

    void DrawCrossHair(UTexture2D* Texture,
                       const FVector2D& ViewportCenter,
                       const FVector2D& Spread,
                       const FLinearColor& Color);

    UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess))
    float CrosshairSpreadMax{ 16.f };

    UPROPERTY(EditAnywhere, Category = "PlayerStats", meta = (AllowAbstract = "false", RuleRangerRequired))
    TSubclassOf<UCharacterOverlay> CharacterOverlayClass{ nullptr };

    UPROPERTY(VisibleAnywhere, Transient, Category = "PlayerStats")
    TObjectPtr<UCharacterOverlay> CharacterOverlay{ nullptr };

    UPROPERTY(EditDefaultsOnly, Category = "Announcements", meta = (AllowAbstract = "false", RuleRangerRequired))
    TSubclassOf<UUserWidget> AnnouncementClass{ nullptr };

    UPROPERTY()
    TObjectPtr<UAnnouncement> Announcement{ nullptr };

public:
    virtual void DrawHUD() override;

    void AddCharacterOverlay();
    void AddAnnouncement();

    FORCEINLINE void SetHUDPackage(const FHUDPackage& InHUDPackage) { HUDPackage = InHUDPackage; }

    FORCEINLINE UCharacterOverlay* GetCharacterOverlay() const { return CharacterOverlay; }
    FORCEINLINE UAnnouncement* GetAnnouncement() const { return Announcement; }
};
