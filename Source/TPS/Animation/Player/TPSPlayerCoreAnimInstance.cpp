#include "TPSPlayerCoreAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "Component/Data/TPSPlayerStateComponent.h"
#include "Pawn/Character/Player/TPSPlayer.h"

void UTPSPlayerCoreAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	if (!OwnerRef.Get())
	{
		OwnerRef = Cast<ATPSPlayer>(TryGetPawnOwner());
		if (ensure(OwnerRef.Get()))
		{
			StateComponentRef = OwnerRef->GetStateComponent();
			ensure(StateComponentRef.Get());
		}
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

		FRotator AimRotation = pOwner->GetBaseAimRotation();
		FRotator ActorRotation = pOwner->GetActorRotation();
		FRotator DeltaRotation = (AimRotation - ActorRotation).GetNormalized();

		AimPitch = DeltaRotation.Pitch;
		AimYaw = DeltaRotation.Yaw;

		UTPSPlayerStateComponent* pStateComp = StateComponentRef.Get();
		if (ensure(pStateComp))
		{
			bIsAiming = pStateComp->HasState(EActionState::Aiming);
		}
	}
}