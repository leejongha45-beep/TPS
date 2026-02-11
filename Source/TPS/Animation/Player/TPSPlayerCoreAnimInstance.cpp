#include "TPSPlayerCoreAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "Pawn/Character/Player/TPSPlayer.h"

void UTPSPlayerCoreAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	GroundSpeed = 0.f;
	
	if (!OwnerRef.Get())
	{
		OwnerRef = Cast<ATPSPlayer>(TryGetPawnOwner());
		ensure(OwnerRef.Get());
	}
}

void UTPSPlayerCoreAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	ATPSPlayer* pOwner = OwnerRef.Get();
	if (ensure(pOwner))
	{
		GroundSpeed = pOwner->GetVelocity().Size2D();
		Direction = UKismetAnimationLibrary::CalculateDirection(pOwner->GetVelocity(), pOwner->GetActorRotation());
	}
}