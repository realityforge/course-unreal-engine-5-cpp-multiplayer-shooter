#include "Weapon/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    if (const auto& OwnerPawn = Cast<APawn>(GetOwner()))
    {
        const auto InstigatorController = OwnerPawn->GetController();

        // TODO: Extract the name out as NAME_MuzzleFlash = FName("MuzzleFlash") somewhere
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

        if (MuzzleFlashSocket && InstigatorController)
        {
            FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
            FVector Start = SocketTransform.GetLocation();
            FVector End = Start + (HitTarget - Start) * 1.25f;

            if (const auto World = GetWorld())
            {
                FHitResult FireHit;
                World->LineTraceSingleByChannel(FireHit, Start, End, ECC_Visibility);
                if (FireHit.bBlockingHit)
                {
                    if (auto Character = Cast<ABlasterCharacter>(FireHit.GetActor()))
                    {
                        if (HasAuthority())
                        {
                            UGameplayStatics::ApplyDamage(Character,
                                                          Damage,
                                                          InstigatorController,
                                                          this,
                                                          UDamageType::StaticClass());
                        }
                    }
                    // It is unclear why the base class does not have an impact callback that we call
                    // from this class and ProjectileWeapon. So then the properties and features could align.
                    // (i.e. Why does this not have a sound?
                    if (ImpactParticles)
                    {
                        UGameplayStatics::SpawnEmitterAtLocation(World,
                                                                 ImpactParticles,
                                                                 FireHit.ImpactPoint,
                                                                 FireHit.ImpactNormal.Rotation());
                    }
                }
            }
        }
    }
}
