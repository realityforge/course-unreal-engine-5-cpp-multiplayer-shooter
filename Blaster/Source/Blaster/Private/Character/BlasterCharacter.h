// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class ABlasterCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABlasterCharacter();

    virtual void Tick(float DeltaTime) override;

    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;

private:
    //---------------------------------------------------------------------------
    // Third Person Camera Section
    //---------------------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    USpringArmComponent* CameraBoom{ nullptr };

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    UCameraComponent* FollowCamera{ nullptr };

    //---------------------------------------------------------------------------

public:
    /** Return the CameraBoom SubObject **/
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    /** Return the FollowCamera SubObject **/
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
