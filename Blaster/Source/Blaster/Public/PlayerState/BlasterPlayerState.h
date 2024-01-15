#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

class ABlasterPlayerController;
class ABlasterCharacter;

UCLASS(Abstract)
class BLASTER_API ABlasterPlayerState final : public APlayerState
{
    GENERATED_BODY()

    /** Cached Character associated with the state. */
    UPROPERTY(Transient)
    TObjectPtr<ABlasterCharacter> Character{ nullptr };

    /** Cached Controller associated with the state. */
    UPROPERTY(Transient)
    TObjectPtr<ABlasterPlayerController> Controller{ nullptr };

    UPROPERTY(ReplicatedUsing = OnRep_Defeats)
    int32 Defeats{ 0 };

    /** Cache and return character associated with the state. */
    ABlasterCharacter* GetCharacter();

    /** Cache and return player controller associated with the state. */
    ABlasterPlayerController* GetController();

    void UpdateScoreOnHUD();

    void UpdateDefeatsOnHUD();

    UFUNCTION()
    void OnRep_Defeats();

public:
    void AddToScore(float ScoreAmount);
    void AddToDefeats(int32 DefeatsAmount);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void OnRep_Score() override;

    FORCEINLINE int32 GetDefeats() const { return Defeats; }
};
