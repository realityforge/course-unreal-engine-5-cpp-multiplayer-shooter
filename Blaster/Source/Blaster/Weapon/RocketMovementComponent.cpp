#include "Weapon/RocketMovementComponent.h"

URocketMovementComponent::EHandleBlockingHitResult
URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit,
                                            const float TimeTick,
                                            const FVector& MoveDelta,
                                            float& SubTickTimeRemaining)
{
    Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);

    // Don't stop moving on collision
    return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
    // Rockets should not stop; only explode when their CollisionBox detects a hit
}
