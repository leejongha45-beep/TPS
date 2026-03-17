#include "TPSLinkedAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

#include "TPSPlayerCoreAnimInstance.h"
#include "Engine/World.h"

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
		bIsFiring = CoreInst->GetIsFiring();
		bIsReloading = CoreInst->GetIsReloading();
		GroundDistance = CoreInst->GetGroundDistance();

		// 신규 미러링
		RootYawOffset = CoreInst->GetRootYawOffset();
		AimYawRate = CoreInst->GetAimYawRate();
		bHasAcceleration = CoreInst->GetHasAcceleration();
		LocalVelocityDirectionAngle = CoreInst->GetLocalVelocityDirectionAngle();
		bIsMovingForward = CoreInst->GetIsMovingForward();
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
