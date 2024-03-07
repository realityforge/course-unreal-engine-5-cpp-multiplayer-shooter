#include "Weapon/Shotgun.h"
#include "BlasterLogging.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
    AWeapon::Fire(HitTarget);
    if (const auto OwnerPawn = Cast<APawn>(GetOwner()))
    {
        const auto InstigatorController = OwnerPawn->GetController();

        if (const auto MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(NAME_MuzzleFlash))
        {
            const FVector Start = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh()).GetLocation();

            TMap<ABlasterCharacter*, uint32> HitMap;
            for (uint32 i = 0; i < NumberOfPellets; i++)
            {
                FHitResult FireHit;
                WeaponTraceHit(Start, HitTarget, FireHit);

                // ReSharper disable once CppTooWideScopeInitStatement
                const auto Character = Cast<ABlasterCharacter>(FireHit.GetActor());
                if (Character && HasAuthority() && InstigatorController)
                {
                    if (HitMap.Contains(Character))
                    {
                        HitMap[Character]++;
                    }
                    else
                    {
                        HitMap.Emplace(Character, 1);
                    }
                }
                if (FireHit.bBlockingHit)
                {
                    if (GetImpactParticles())
                    {
                        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
                                                                 GetImpactParticles(),
                                                                 FireHit.ImpactPoint,
                                                                 FireHit.ImpactNormal.Rotation());
                    }
                    if (GetHitSound())
                    {

                        UGameplayStatics::PlaySoundAtLocation(this,
                                                              GetHitSound(),
                                                              FireHit.ImpactPoint,
                                                              .5f,
                                                              FMath::FRandRange(-.5f, .5f));
                    }
                }
            }
            if (HasAuthority() && InstigatorController)
            {
                for (const auto CharacterHit : HitMap)
                {
                    if (CharacterHit.Key)
                    {
                        const float TotalDamage = GetDamage() * CharacterHit.Value;
                        UGameplayStatics::ApplyDamage(CharacterHit.Key,
                                                      TotalDamage,
                                                      InstigatorController,
                                                      this,
                                                      UDamageType::StaticClass());
                    }
                }
            }
        }
    }
}
