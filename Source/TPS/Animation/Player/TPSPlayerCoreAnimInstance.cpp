#include "TPSPlayerCoreAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "Component/Action/TPSEquipComponent.h"
#include "Component/Data/TPSPlayerStateComponent.h"
#include "Pawn/Character/Player/TPSPlayer.h"

void UTPSPlayerCoreAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (!OwnerRef.Get())
	{
		OwnerRef = Cast<ATPSPlayer>(TryGetPawnOwner());
		if (ensure(OwnerRef.Get()))
		{
			StateComponentRef = OwnerRef->GetStateComponent();
			ensure(StateComponentRef.Get());

			UTPSEquipComponent* pEquipComp = OwnerRef->FindComponentByClass<UTPSEquipComponent>();
			if (ensure(pEquipComp))
			{
				EquipComponentRef = pEquipComp;
				pEquipComp->OnEquipMontagePlayDelegate.AddUObject(this, &UTPSPlayerCoreAnimInstance::PlayEquipMontage);
			}
		}
	}
}

void UTPSPlayerCoreAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ATPSPlayer* pOwner = OwnerRef.Get();
	if (!pOwner)
	{
		return;
	}

	// 게임 스레드: UObject 접근이 필요한 원시 데이터 캐싱
	CachedVelocity = pOwner->GetVelocity();
	CachedActorRotation = pOwner->GetActorRotation();
	CachedAimRotation = pOwner->GetBaseAimRotation();

	// 비원자적 읽기 → 게임 스레드 전용
	UTPSPlayerStateComponent* pStateComp = StateComponentRef.Get();
	if (ensure(pStateComp))
	{
		bIsAiming = pStateComp->HasState(EActionState::Aiming);
		bIsEquipping = pStateComp->HasState(EActionState::Equipping);
		bIsFalling = pStateComp->HasState(EActionState::Falling);
	}

	// GroundDistance: 라인 트레이스(물리 월드 접근) → 게임 스레드 전용
	if (bIsFalling)
	{
		const FVector Start = pOwner->GetActorLocation();
		const FVector End = Start - FVector(0.f, 0.f, 10000.f);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(pOwner);

		if (pOwner->GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
		{
			GroundDistance = HitResult.Distance;
		}
		else
		{
			GroundDistance = 10000.f;
		}
	}
	else
	{
		GroundDistance = 0.f;
	}
}

void UTPSPlayerCoreAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	// 워커 스레드: 캐시된 값 타입으로 순수 수학 연산만 수행
	GroundSpeed = CachedVelocity.Size2D();
	Direction = UKismetAnimationLibrary::CalculateDirection(CachedVelocity, CachedActorRotation);

	FRotator DeltaRotation = (CachedAimRotation - CachedActorRotation).GetNormalized();
	AimPitch = DeltaRotation.Pitch;
	AimYaw = DeltaRotation.Yaw;
}

void UTPSPlayerCoreAnimInstance::PlayEquipMontage(bool bEquip)
{
	UAnimMontage* pMontage = bEquip ? EquipMontageAsset : UnequipMontageAsset;
	if (!ensure(pMontage)) return;

	Montage_Play(pMontage);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UTPSPlayerCoreAnimInstance::OnEquipMontageEnded);
	Montage_SetEndDelegate(EndDelegate, pMontage);
}

void UTPSPlayerCoreAnimInstance::OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UTPSEquipComponent* pEquipComp = EquipComponentRef.Get();
	if (!ensure(pEquipComp)) return;

	if (bInterrupted)
	{
		pEquipComp->OnMontageInterrupted();
		return;
	}

	pEquipComp->OnMontageFinished(!bIsEquipping);
}