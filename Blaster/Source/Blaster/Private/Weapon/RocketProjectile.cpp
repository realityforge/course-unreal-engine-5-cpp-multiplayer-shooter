#include "Weapon/RocketProjectile.h"
#include "BlasterLogging.h"
#include "Kismet/GameplayStatics.h"

static FName RocketMeshComponentName(TEXT("RocketMesh"));

ARocketProjectile::ARocketProjectile()
{
    RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(RocketMeshComponentName);
    RocketMesh->SetupAttachment(RootComponent);
    RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ARocketProjectile::OnHit(UPrimitiveComponent* HitComp,
                              AActor* OtherActor,
                              UPrimitiveComponent* OtherComp,
                              const FVector NormalImpulse,
                              const FHitResult& Hit)
{
    AController* InstigatorController{ nullptr };
    if (const auto& DamageInstigator = GetInstigator())
    {
        InstigatorController = DamageInstigator->GetController();
    }
    if (!InstigatorController)
    {
        BL_ULOG_WARNING("Unable to determine InstigatorController from Instigator '%s'",
                        GetInstigator() ? *GetInstigator()->GetName() : TEXT("?"))
    }
    UGameplayStatics::ApplyRadialDamageWithFalloff(this,                       // World context object
                                                   GetDamage(),                // BaseDamage
                                                   10.f,                       // MinimumDamage
                                                   GetActorLocation(),         // Origin
                                                   200.f,                      // DamageInnerRadius
                                                   500.f,                      // DamageOuterRadius
                                                   1.f,                        // DamageFalloff
                                                   UDamageType::StaticClass(), // DamageTypeClass
                                                   TArray<AActor*>(),          // IgnoreActors
                                                   this,                       // DamageCauser
                                                   InstigatorController        // InstigatorController
    );

    Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
