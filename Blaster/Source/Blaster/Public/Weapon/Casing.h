#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS(Abstract)
class BLASTER_API ACasing : public AActor
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* CasingMesh{ nullptr };

protected:
    virtual void BeginPlay() override;

public:
    ACasing();

    virtual void Tick(float DeltaTime) override;
};
