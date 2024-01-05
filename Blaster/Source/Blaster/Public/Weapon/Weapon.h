// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class ACasing;
class UWidgetComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    EWS_Initial UMETA(DisplayString = "Initial State"),
    EWS_Equipped UMETA(DisplayString = "Equipped"),
    EWS_Dropped UMETA(DisplayString = "Dropped"),

    EWS_MAX UMETA(Hidden)
};

UCLASS(Abstract, MinimalAPI)
class AWeapon : public AActor
{
    GENERATED_BODY()

public:
    AWeapon();

    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;

    void ShowPickupWidget(bool bShowWidget) const;

    void SetWeaponState(const EWeaponState InWeaponState);

    virtual void Fire(const FVector& HitTarget);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnAreaSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                          AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp,
                                          int32 OtherBodyIndex,
                                          bool bFromSweep,
                                          const FHitResult& SweepResult);

    UFUNCTION()
    virtual void OnAreaSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                        AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp,
                                        int32 OtherBodyIndex);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess))
    TObjectPtr<USkeletalMeshComponent> WeaponMesh;

    UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (AllowPrivateAccess))
    TObjectPtr<USphereComponent> AreaSphere;

    UPROPERTY(ReplicatedUsing = OnRep_WeaponState,
              VisibleAnywhere,
              Category = "Weapon Properties",
              meta = (AllowPrivateAccess))
    EWeaponState WeaponState{ EWeaponState::EWS_Initial };

    /** Called when WeaponState has been replicated. */
    UFUNCTION()
    void OnRep_WeaponState() const;

    /** Called on both server and client to represent actions that must occur on both sides. */
    void OnWeaponStateUpdated() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess))
    TObjectPtr<UWidgetComponent> PickupWidget{ nullptr };

    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = "Weapon Properties",
              meta = (AllowPrivateAccess, RuleRangerRequired))
    TObjectPtr<UAnimationAsset> FireAnimation{ nullptr };

    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = "Weapon Properties",
              meta = (AllowAbstract = "false", AllowPrivateAccess, RuleRangerRequired))
    TSubclassOf<ACasing> CasingClass{ nullptr };

    //---------------------------------------------------------------------------
    // Textures for the Crosshairs
    //---------------------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = Crosshairs, meta = (RuleRangerRequired))
    UTexture2D* CrosshairsCenter{ nullptr };
    UPROPERTY(EditDefaultsOnly, Category = Crosshairs, meta = (RuleRangerRequired))
    UTexture2D* CrosshairsLeft{ nullptr };
    UPROPERTY(EditDefaultsOnly, Category = Crosshairs, meta = (RuleRangerRequired))
    UTexture2D* CrosshairsRight{ nullptr };
    UPROPERTY(EditDefaultsOnly, Category = Crosshairs, meta = (RuleRangerRequired))
    UTexture2D* CrosshairsTop{ nullptr };
    UPROPERTY(EditDefaultsOnly, Category = Crosshairs, meta = (RuleRangerRequired))
    UTexture2D* CrosshairsBottom{ nullptr };

    //---------------------------------------------------------------------------
    // Zoomed FOV while aiming
    //---------------------------------------------------------------------------

    /** The FOV when zoomed in using weapon. */
    UPROPERTY(EditDefaultsOnly, Category = "Aiming")
    float ZoomedFOV{ 30.f };

    /** The speed at which to interp to or from ZoomFOV. */
    UPROPERTY(EditDefaultsOnly, Category = "Aiming")
    float ZoomInterpSpeed{ 20.f };

public:
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; };
    FORCEINLINE UTexture2D* GetCrosshairsCenter() const { return CrosshairsCenter; };
    FORCEINLINE UTexture2D* GetCrosshairsLeft() const { return CrosshairsLeft; };
    FORCEINLINE UTexture2D* GetCrosshairsRight() const { return CrosshairsRight; };
    FORCEINLINE UTexture2D* GetCrosshairsTop() const { return CrosshairsTop; };
    FORCEINLINE UTexture2D* GetCrosshairsBottom() const { return CrosshairsBottom; };
    FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
    FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
};
