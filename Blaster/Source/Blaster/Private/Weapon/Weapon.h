// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class USphereComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    EWS_Initial UMETA(DisplayString = "Initial State"),
    EWS_Equipped UMETA(DisplayString = "Equipped"),
    EWS_Dropped UMETA(DisplayString = "Dropped"),

    EWS_MAX UMETA(Hidden)
};

UCLASS()
class AWeapon : public AActor
{
    GENERATED_BODY()

public:
    AWeapon();

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (AllowPrivateAccess))
    TObjectPtr<USkeletalMeshComponent> WeaponMesh;

    UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (AllowPrivateAccess))
    TObjectPtr<USphereComponent> AreaSphere;

    UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (AllowPrivateAccess))
    EWeaponState WeaponState;
};
