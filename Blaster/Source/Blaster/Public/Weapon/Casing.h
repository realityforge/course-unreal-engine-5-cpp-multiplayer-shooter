#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

class USoundCue;

UCLASS(Abstract)
class BLASTER_API ACasing final : public AActor
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* CasingMesh{ nullptr };

    UPROPERTY(EditDefaultsOnly)
    USoundCue* ShellSound{ nullptr };

    UPROPERTY(EditDefaultsOnly)
    float InitialImpulse{ 10.f };

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent,
                       AActor* OtherActor,
                       UPrimitiveComponent* OtherComponent,
                       FVector NormalImpulse,
                       const FHitResult& Hit);

public:
    ACasing();

    virtual void Tick(float DeltaTime) override;
};
