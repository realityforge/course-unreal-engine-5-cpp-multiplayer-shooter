#include "Weapon/ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    // Only spawn the projectile on the server and then make sure we replicate to client
    if (HasAuthority())
    {
        if (const auto MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash")))
        {
            const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
            // From MuzzleFlash socket to HitTarget
            const FVector MuzzleFlashToHitTarget = HitTarget - SocketTransform.GetLocation();
            if (ensure(ProjectileClass))
            {
                if (const auto World = GetWorld())
                {
                    FActorSpawnParameters SpawnParams;
                    // The owner of the weapon is the character that fired the weapon so we
                    // set the owner of the projectile to the same owner
                    SpawnParams.Owner = GetOwner();
                    // The instigator is also the character (just cast to Pawn)
                    SpawnParams.Instigator = GetInstigator();

                    // Spawn the projectile at MuzzleFlash, oriented towards the HitTarget
                    World->SpawnActor<AProjectile>(ProjectileClass,
                                                   SocketTransform.GetLocation(),
                                                   MuzzleFlashToHitTarget.Rotation(),
                                                   SpawnParams);
                }
            }
        }
    }
}
