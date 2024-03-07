#pragma once

#include "Character/BlasterCharacter.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

class ABlasterPlayerController;
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

    void UpdateHUDAmmo();
    void ClearCachedOwnerProperties();

protected:
    static const inline FName NAME_MuzzleFlash = FName("MuzzleFlash");

    virtual void OnRep_Owner() override;

public:
    /**
     * Called to transition object to dropped state.
     */
    void Dropped();

    /**
     * Add Ammo to the current Ammo.
     * The result current Ammo has a max value and if Add exceeds it we will ignore additional values.
     *
     * @param InAmmo The amount of Ammo to add.
     */
    void AddAmmo(int32 InAmmo);

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
    TObjectPtr<USkeletalMeshComponent> WeaponMesh{ nullptr };

    UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (AllowPrivateAccess))
    TObjectPtr<USphereComponent> AreaSphere{ nullptr };

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess))
    TObjectPtr<UAnimationAsset> FireAnimation{ nullptr };

    UPROPERTY(EditDefaultsOnly,
              BlueprintReadOnly,
              Category = "Weapon Properties",
              meta = (AllowAbstract = "false", AllowPrivateAccess))
    TSubclassOf<ACasing> CasingClass{ nullptr };

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess))
    EWeaponType WeaponType{ EWeaponType::AssaultRifle };

    /** Sound effect when we equip the weapon. */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties", meta = (RuleRangerRequired))
    TObjectPtr<USoundCue> EquipSound;

    //---------------------------------------------------------------------------
    // Textures for the Crosshairs
    //---------------------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = Crosshairs, meta = (RuleRangerRequired))
    TObjectPtr<UTexture2D> CrosshairsCenter{ nullptr };
    UPROPERTY(EditDefaultsOnly, Category = Crosshairs, meta = (RuleRangerRequired))
    TObjectPtr<UTexture2D> CrosshairsLeft{ nullptr };
    UPROPERTY(EditDefaultsOnly, Category = Crosshairs, meta = (RuleRangerRequired))
    TObjectPtr<UTexture2D> CrosshairsRight{ nullptr };
    UPROPERTY(EditDefaultsOnly, Category = Crosshairs)
    TObjectPtr<UTexture2D> CrosshairsTop{ nullptr };
    UPROPERTY(EditDefaultsOnly, Category = Crosshairs)
    TObjectPtr<UTexture2D> CrosshairsBottom{ nullptr };

    //---------------------------------------------------------------------------
    // Zoomed FOV while aiming
    //---------------------------------------------------------------------------

    /** The FOV when zoomed in using weapon. */
    UPROPERTY(EditDefaultsOnly, Category = "Aiming")
    float ZoomedFOV{ 30.f };

    /** The speed at which to interp to or from ZoomFOV. */
    UPROPERTY(EditDefaultsOnly, Category = "Aiming")
    float ZoomInterpSpeed{ 20.f };

    //---------------------------------------------------------------------------
    // Automatic fire
    //---------------------------------------------------------------------------

    UPROPERTY(EditAnywhere, Category = "Combat")
    float FireDelay{ 0.15f };

    UPROPERTY(EditAnywhere, Category = "Combat")
    bool bAutomaticFire{ true };

    //---------------------------------------------------------------------------
    // Ammo Management
    //---------------------------------------------------------------------------

    /** The maximum amount of ammo that a weapon can contain. */
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    int32 MaxAmmoCapacity{ 10 };

    /** The amount of Ammo that is currently available. */
    UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo, Category = "Combat")
    int32 Ammo{ 10 };

    UFUNCTION()
    void OnRep_Ammo();

    void UseAmmo();

    //---------------------------------------------------------------------------
    // Cached Reference Management. (Shouldn't be needed if was designed better).
    //---------------------------------------------------------------------------

    UPROPERTY(Transient)
    TObjectPtr<ABlasterCharacter> OwnerCharacter{ nullptr };

    UPROPERTY(Transient)
    TObjectPtr<ABlasterPlayerController> OwnerController{ nullptr };

    /** Cache and retrieve Character. */
    ABlasterCharacter* GetOwnerCharacter();

    /** Cache and retrieve Controller. */
    ABlasterPlayerController* GetOwnerController();

public:
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; };
    FORCEINLINE UTexture2D* GetCrosshairsCenter() const { return CrosshairsCenter; };
    FORCEINLINE UTexture2D* GetCrosshairsLeft() const { return CrosshairsLeft; };
    FORCEINLINE UTexture2D* GetCrosshairsRight() const { return CrosshairsRight; };
    FORCEINLINE UTexture2D* GetCrosshairsTop() const { return CrosshairsTop; };
    FORCEINLINE UTexture2D* GetCrosshairsBottom() const { return CrosshairsBottom; };
    FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; };
    FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
    FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
    FORCEINLINE float GetFireDelay() const { return FireDelay; }
    FORCEINLINE bool IsAutomaticFire() const { return bAutomaticFire; }
    FORCEINLINE bool HasAmmo() const { return Ammo > 0; }
    FORCEINLINE int32 GetAmmo() const { return Ammo; }
    FORCEINLINE int32 GetMaxAmmoCapacity() const { return MaxAmmoCapacity; }
    FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
};
