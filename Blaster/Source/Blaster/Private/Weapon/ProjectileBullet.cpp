// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/ProjectileBullet.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent,
                              AActor* OtherActor,
                              UPrimitiveComponent* OtherComponent,
                              FVector NormalImpulse,
                              const FHitResult& Hit)
{
    // Owner chain gos from projectile to weapon to character using weapon
    if (const auto OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        if (AController* OwnerController = OwnerCharacter->Controller)
        {
            UGameplayStatics::ApplyDamage(OtherActor, GetDamage(), OwnerController, this, UDamageType::StaticClass());
        }
    }
    Super::OnHit(HitComponent, OtherActor, OtherComponent, NormalImpulse, Hit);
}
