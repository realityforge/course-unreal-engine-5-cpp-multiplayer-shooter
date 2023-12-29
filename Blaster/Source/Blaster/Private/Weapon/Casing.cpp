#include "Weapon/Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
    PrimaryActorTick.bCanEverTick = false;
    CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
    // Stop a camera jumping in (due to occlusion) when the a casing flies between the camera and the character
    CasingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    CasingMesh->SetSimulatePhysics(true);
    CasingMesh->SetEnableGravity(true);

    // Make sure we get OnHit called
    CasingMesh->SetNotifyRigidBodyCollision(true);

    SetRootComponent(CasingMesh);
}

void ACasing::BeginPlay()
{
    Super::BeginPlay();
    CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
    CasingMesh->AddImpulse(GetActorForwardVector() * InitialImpulse);
}

void ACasing::OnHit(UPrimitiveComponent* HitComponent,
                    AActor* OtherActor,
                    UPrimitiveComponent* OtherComponent,
                    FVector NormalImpulse,
                    const FHitResult& Hit)
{
    if (ShellSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
    }
    Destroy();
}

void ACasing::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
