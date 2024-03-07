#include "Weapon/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    if (const auto& OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (const auto MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(NAME_MuzzleFlash))
        {
            FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
            FVector Start = SocketTransform.GetLocation();

            FHitResult FireHit;
            WeaponTraceHit(Start, HitTarget, FireHit);
            if (FireHit.bBlockingHit)
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                const auto InstigatorController = OwnerPawn->GetController();
                // ReSharper disable once CppTooWideScopeInitStatement
                auto Character = Cast<ABlasterCharacter>(FireHit.GetActor());
                if (Character && HasAuthority() && InstigatorController)
                {
                    UGameplayStatics::ApplyDamage(Character,
                                                  Damage,
                                                  InstigatorController,
                                                  this,
                                                  UDamageType::StaticClass());
                }
                // It is unclear why the base class does not have an impact callback that we call
                // from this class and ProjectileWeapon. So then the properties and features could
                // align when we have impacts. Now just compy impact and sound events (also this is
                // more expensive over rpc as we collapse these messages into destroy rpc event for
                // projectiles)
                if (ImpactParticles)
                {
                    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
                                                             ImpactParticles,
                                                             FireHit.ImpactPoint,
                                                             FireHit.ImpactNormal.Rotation());
                }
                if (HitSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
                }
            }
            if (MuzzleFlash)
            {
                // If Muzzle flash not part of firing animation then explicitly spawn emitter
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
            }
            if (FireSound)
            {
                // If firing sound is not part of firing animation then explicitly play sound
                UGameplayStatics::PlaySoundAtLocation(this, FireSound, SocketTransform.GetLocation());
            }
        }
    }
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget) const
{
    // Derive direction from start to target as a normalized vector
    const FVector Direction = (HitTarget - TraceStart).GetSafeNormal();

    const FVector SphereCenter = TraceStart + Direction * DistanceToSphere;
    // Randomize a location within the sphere
    const FVector RandomSphereOffset = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);

    const FVector EndLocation = SphereCenter + RandomSphereOffset;
    const FVector StartToEndDirection = EndLocation - TraceStart;
    const FVector TraceEnd = TraceStart + StartToEndDirection * TARGETING_RANGE / StartToEndDirection.Size();

    if (false)
    {
        // Draw the sphere that defines scatter pattern
        DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, false, 2.f);
        // Orange sphere is the point in the sphere that the shot goes through
        DrawDebugSphere(GetWorld(), EndLocation, 4.f, 12, FColor::Orange, false, 2.f);
        // The line for weapon trace
        DrawDebugLine(GetWorld(), TraceStart, FVector(TraceEnd), FColor::Cyan, false, 2.f);
    }

    return TraceEnd;
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit) const
{
    if (const auto World = GetWorld())
    {
        const FVector End =
            bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;

        World->LineTraceSingleByChannel(OutHit, TraceStart, End, ECC_Visibility);
        FVector BeamEnd = End;
        if (OutHit.bBlockingHit)
        {
            BeamEnd = OutHit.ImpactPoint;
        }
        if (BeamParticles)
        {
            if (const auto Beam = UGameplayStatics::SpawnEmitterAtLocation(World,
                                                                           BeamParticles,
                                                                           TraceStart,
                                                                           FRotator::ZeroRotator,
                                                                           true))
            {
                static const FName NAME_Target = FName("Target");
                Beam->SetVectorParameter(NAME_Target, BeamEnd);
            }
        }
    }
}
