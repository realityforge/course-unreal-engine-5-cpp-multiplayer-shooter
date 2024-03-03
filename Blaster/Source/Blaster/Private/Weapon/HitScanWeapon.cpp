#include "Weapon/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    if (const auto& OwnerPawn = Cast<APawn>(GetOwner()))
    {
        // ReSharper disable once CppUE4CodingStandardNamingViolationWarning
        static const FName NAME_MuzzleFlash = FName("MuzzleFlash");
        if (const auto MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(NAME_MuzzleFlash))
        {
            FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
            FVector Start = SocketTransform.GetLocation();
            FVector End = Start + (HitTarget - Start) * 1.25f;

            if (const auto World = GetWorld())
            {
                FHitResult FireHit;
                FVector BeamEnd = End;
                World->LineTraceSingleByChannel(FireHit, Start, End, ECC_Visibility);
                if (FireHit.bBlockingHit)
                {
                    BeamEnd = FireHit.ImpactPoint;
                    if (auto Character = Cast<ABlasterCharacter>(FireHit.GetActor()))
                    {
                        // ReSharper disable once CppTooWideScopeInitStatement
                        const auto InstigatorController = OwnerPawn->GetController();
                        if (HasAuthority() && InstigatorController)
                        {
                            UGameplayStatics::ApplyDamage(Character,
                                                          Damage,
                                                          InstigatorController,
                                                          this,
                                                          UDamageType::StaticClass());
                        }
                    }
                    // It is unclear why the base class does not have an impact callback that we call
                    // from this class and ProjectileWeapon. So then the properties and features could
                    // align when we have impacts. Now just compy impact and sound events (also this is
                    // more expensive over rpc as we collapse these messages into destroy rpc event for
                    // projectiles)
                    if (ImpactParticles)
                    {
                        UGameplayStatics::SpawnEmitterAtLocation(World,
                                                                 ImpactParticles,
                                                                 FireHit.ImpactPoint,
                                                                 FireHit.ImpactNormal.Rotation());
                    }
                    if (HitSound)
                    {
                        UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
                    }
                }
                if (BeamParticles)
                {
                    if (auto Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, SocketTransform))
                    {
                        Beam->SetVectorParameter(FName("Target"), BeamEnd);
                    }
                }
                if (MuzzleFlash)
                {
                    // If Muzzle flash not part of firing animation then explicitly spawn emitter
                    UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleFlash, SocketTransform);
                }
                if (FireSound)
                {
                    // If firing sound is not part of firing animation then explicitly play sound
                    UGameplayStatics::PlaySoundAtLocation(this, FireSound, SocketTransform.GetLocation());
                }
            }
        }
    }
}
