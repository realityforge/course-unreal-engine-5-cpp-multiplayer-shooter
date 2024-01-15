#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "OverheadWidget.generated.h"

class UTextBlock;

UCLASS()
class UOverheadWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DisplayText;

    /**
     * Set the text on DisplayText widget to supplied value.
     *
     * @param TextToDisplay the text value.
     */
    void SetDisplayText(const FString& TextToDisplay) const;

    UFUNCTION(BlueprintCallable)
    void ShowPlayerNetRole(const APawn* InPawn) const;

protected:
    virtual void NativeDestruct() override;
};
