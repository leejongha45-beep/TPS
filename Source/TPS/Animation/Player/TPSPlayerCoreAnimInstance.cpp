#include "TPSPlayerCoreAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Component/Action/TPSEquipComponent.h"
#include "Character/Component/Action/TPSFireComponent.h"
#include "Character/Component/Data/TPSPlayerStateComponent.h"
#include "Character/Player/TPSPlayer.h"

void UTPSPlayerCoreAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (!OwnerRef.Get())
	{
		// ① 오너 캐싱 + 컴포넌트 참조 획득
		OwnerRef = Cast<ATPSPlayer>(TryGetPawnOwner());
		if (ensure(OwnerRef.Get()))
		{
			StateComponentRef = OwnerRef->GetStateComponent();
			ensure(StateComponentRef.Get());

			// ② EquipComponent 몽타주 델리게이트 바인딩
			UTPSEquipComponent* pEquipComp = OwnerRef->GetEquipComponent();
			if (ensure(pEquipComp))
			{
				EquipComponentRef = pEquipComp;
				if (!pEquipComp->OnEquipMontagePlayDelegate.IsBoundToObject(this))
				{
					pEquipComp->OnEquipMontagePlayDelegate.AddUObject(this, &UTPSPlayerCoreAnimInstance::PlayEquipMontage);
				}
			}

			// ③ FireComponent 몽타주 델리게이트 바인딩
			UTPSFireComponent* pFireComp = OwnerRef->GetFireComponent();
			if (ensure(pFireComp))
			{
				FireComponentRef = pFireComp;
				if (!pFireComp->OnFireStateChangedDelegate.IsBoundToObject(this))
				{
					pFireComp->OnFireStateChangedDelegate.AddUObject(this, &UTPSPlayerCoreAnimInstance::OnFireStateChanged);
				}
				pFireComp->OnFireOnceDelegate.BindUObject(this, &UTPSPlayerCoreAnimInstance::OnFireOnce);

				// ④ FireComponent 재장전 몽타주 델리게이트 바인딩
				if (!pFireComp->OnReloadMontagePlayDelegate.IsBoundToObject(this))
				{
					pFireComp->OnReloadMontagePlayDelegate.AddUObject(this, &UTPSPlayerCoreAnimInstance::PlayReloadMontage);
				}
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

	// ① 게임 스레드: UObject 접근이 필요한 원시 데이터 캐싱
	CachedVelocity = pOwner->GetVelocity();
	CachedActorRotation = pOwner->GetActorRotation();
	CachedAimRotation = pOwner->GetBaseAimRotation();

	// ② 비원자적 읽기 → 게임 스레드 전용 (StateComponent)
	UTPSPlayerStateComponent* pStateComp = StateComponentRef.Get();
	if (ensure(pStateComp))
	{
		bIsAiming = pStateComp->HasState(EActionState::Aiming);
		bIsEquipping = pStateComp->HasState(EActionState::Equipping);
		bIsFalling = pStateComp->HasState(EActionState::Falling);
		bIsFiring = pStateComp->HasState(EActionState::Firing);
		bIsReloading = pStateComp->HasState(EActionState::Reloading);
	}

	// ③ 가속 감지 (CMC 접근)
	const UCharacterMovementComponent* CMC = pOwner->GetCharacterMovement();
	if (CMC)
	{
		bHasAcceleration = !CMC->GetCurrentAcceleration().IsNearlyZero();
	}

	// ④ Root Yaw Offset (Turn in Place)
	const float CurrentControllerYaw = pOwner->GetControlRotation().Yaw;
	const float ControllerYawDelta = FRotator::NormalizeAxis(CurrentControllerYaw - PreviousControllerYaw);

	if (bIsEquipping)
	{
		// 장착 상태: RootYawOffset을 0으로 보간 (카메라 방향 고정)
		RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.f, DeltaSeconds, 10.f);
	}
	else
	{
		switch (RootYawOffsetMode)
		{
		case ERootYawOffsetMode::Accumulate:
			// Idle — 컨트롤러 회전의 역방향으로 누적
			RootYawOffset = FRotator::NormalizeAxis(RootYawOffset - ControllerYawDelta);
			break;
		case ERootYawOffsetMode::BlendOut:
			// Jog — 부드럽게 0으로 보간
			RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.f, DeltaSeconds, 10.f);
			break;
		case ERootYawOffsetMode::Hold:
			// Start — 현재 값 유지
			break;
		}
	}

	PreviousControllerYaw = CurrentControllerYaw;

	// ⑤ 상체 블렌드 가중치 (장착 상태이거나 장착/해제/발사/재장전 몽타주 재생 중)
	const bool bIsPlayingEquipMontage =
		(EquipMontageAsset && Montage_IsPlaying(EquipMontageAsset)) ||
		(UnequipMontageAsset && Montage_IsPlaying(UnequipMontageAsset));
	const bool bIsPlayingFireMontage =
		(FireMontageAsset && Montage_IsPlaying(FireMontageAsset));
	const bool bIsPlayingReloadMontage =
		(ReloadMontageAsset && Montage_IsPlaying(ReloadMontageAsset));
	UpperBodyBlendWeight = (bIsEquipping || bIsPlayingEquipMontage || bIsPlayingFireMontage || bIsPlayingReloadMontage) ? 1.f : 0.f;

	// ⑥ GroundDistance: 라인 트레이스(물리 월드 접근) → 게임 스레드 전용
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

	// ① GroundSpeed + Direction 계산 (캐시된 값 타입 → 순수 수학)
	GroundSpeed = CachedVelocity.Size2D();
	Direction = UKismetAnimationLibrary::CalculateDirection(CachedVelocity, CachedActorRotation);

	// ② AimYaw/AimPitch + AimYawRate
	const float PreviousAimYaw = AimYaw;
	FRotator DeltaRotation = (CachedAimRotation - CachedActorRotation).GetNormalized();
	AimPitch = DeltaRotation.Pitch;
	AimYaw = DeltaRotation.Yaw;
	AimYawRate = (DeltaSeconds > SMALL_NUMBER)
		? FMath::Abs(FRotator::NormalizeAxis(AimYaw - PreviousAimYaw)) / DeltaSeconds
		: 0.f;

	// ③ 이동 방향 각도
	if (GroundSpeed > 5.f)
	{
		LocalVelocityDirectionAngle = Direction;
		bIsMovingForward = FMath::Abs(LocalVelocityDirectionAngle) < 60.f;
	}
	else
	{
		bIsMovingForward = false;
	}
}

void UTPSPlayerCoreAnimInstance::PlayEquipMontage(bool bEquip)
{
	// ① Equip/Unequip 몽타주 선택
	UAnimMontage* pMontage = bEquip ? EquipMontageAsset : UnequipMontageAsset;
	if (!ensure(pMontage)) return;

	// ② 몽타주 재생 + 종료 델리게이트 바인딩
	Montage_Play(pMontage);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UTPSPlayerCoreAnimInstance::OnEquipMontageEnded);
	Montage_SetEndDelegate(EndDelegate, pMontage);
}

void UTPSPlayerCoreAnimInstance::OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UTPSEquipComponent* pEquipComp = EquipComponentRef.Get();
	if (!ensure(pEquipComp)) return;

	// 중간에 끊길 경우
	if (bInterrupted)
	{
		pEquipComp->OnMontageInterrupted();
		return;
	}

	pEquipComp->OnMontageFinished(!bIsEquipping);
}

void UTPSPlayerCoreAnimInstance::OnFireStateChanged(bool bFiring)
{
	if (bFiring) return;

	// 발사 종료: 몽타주 정지 (블렌드 아웃)
	if (FireMontageAsset && Montage_IsPlaying(FireMontageAsset))
	{
		Montage_Stop(0.25f, FireMontageAsset);
	}
}

void UTPSPlayerCoreAnimInstance::OnFireOnce()
{
	if (!ensure(FireMontageAsset)) return;

	// 발사 1회: 몽타주 처음부터 재생 (연사 시 매번 리셋)
	Montage_Play(FireMontageAsset, 1.f, EMontagePlayReturnType::MontageLength, 0.f);
}

void UTPSPlayerCoreAnimInstance::PlayReloadMontage()
{
	if (!ensure(ReloadMontageAsset)) return;

	// ① 몽타주 재생 + 종료 델리게이트 바인딩
	Montage_Play(ReloadMontageAsset);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UTPSPlayerCoreAnimInstance::OnReloadMontageEnded);
	Montage_SetEndDelegate(EndDelegate, ReloadMontageAsset);
}

void UTPSPlayerCoreAnimInstance::OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UTPSFireComponent* pFireComp = FireComponentRef.Get();
	if (!ensure(pFireComp)) return;

	pFireComp->OnReloadMontageFinished(bInterrupted);
}

void UTPSPlayerCoreAnimInstance::SetRootYawOffsetMode(ERootYawOffsetMode InMode)
{
	RootYawOffsetMode = InMode;
}