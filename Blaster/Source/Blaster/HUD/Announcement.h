#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Announcement.generated.h"

class UTextBlock;

UCLASS(Abstract)
class BLASTER_API UAnnouncement : public UUserWidget
{
    GENERATED_BODY()

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> WarmupTime{ nullptr };

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> AnnouncementText{ nullptr };

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> InfoText{ nullptr };

public:
    FORCEINLINE UTextBlock* GetWarmupTime() const { return WarmupTime; }
    FORCEINLINE UTextBlock* GetAnnouncementText() const { return AnnouncementText; }
    FORCEINLINE UTextBlock* GetInfoText() const { return InfoText; }
};
