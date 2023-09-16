// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
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

    //---------------------------------------------------------------------------
    // User Inputs Section
    //---------------------------------------------------------------------------

    /** Called for movement input */
    void Move(const FInputActionValue& Value);

    /** Called for looking input */
    void Look(const FInputActionValue& Value);

    //---------------------------------------------------------------------------

private:
    //---------------------------------------------------------------------------
    // User Inputs Section
    //---------------------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "Character Input", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<UInputMappingContext> InputMapping{ nullptr };

    /** Jump Input Action */
    UPROPERTY(EditDefaultsOnly, Category = "Character Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction{ nullptr };

    /** Move Input Action */
    UPROPERTY(EditDefaultsOnly, Category = "Character Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction{ nullptr };

    /** Look Input Action */
    UPROPERTY(EditDefaultsOnly, Category = "Character Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction{ nullptr };

    //---------------------------------------------------------------------------

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
