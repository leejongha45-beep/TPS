#include "TPSLinkedAnimInstance.h"

#include "TPSPlayerCoreAnimInstance.h"

void UTPSLinkedAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (const UTPSPlayerCoreAnimInstance* CoreInst = GetCoreAnimInstance())
	{
		GroundSpeed = CoreInst->GetGroundSpeed();
		Direction = CoreInst->GetDirection();
		AimPitch = CoreInst->GetAimPitch();
		AimYaw = CoreInst->GetAimYaw();
		bIsAiming = CoreInst->GetIsAiming();
		bIsEquipping = CoreInst->GetIsEquipping();
		bIsFalling = CoreInst->GetIsFalling();
		GroundDistance = CoreInst->GetGroundDistance();
	}
}

UTPSPlayerCoreAnimInstance* UTPSLinkedAnimInstance::GetCoreAnimInstance() const
{
	if (const USkeletalMeshComponent* OwningComp = GetOwningComponent())
	{
		return Cast<UTPSPlayerCoreAnimInstance>(OwningComp->GetAnimInstance());
	}
	return nullptr;
}
