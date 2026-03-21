#include "TPSFireComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Weapon/Projectile/TPSProjectilePoolSubsystem.h"
#include "Weapon/TPSWeaponBase.h"
#include "Weapon/Projectile/TPSProjectileBase.h"

DECLARE_LOG_CATEGORY_EXTERN(FireLog, Log, All);

DEFINE_LOG_CATEGORY(FireLog);

void UTPSFireComponent::StartFire(ATPSWeaponBase* InWeapon, TFunction<void (FVector&, FRotator&)> InViewPointGetter)
{
	if (bIsFiring) return;
	if (bIsReloading) return;
	if (!ensure(InWeapon)) return;

	// ① 무기 참조 캐싱 + 탄약 확인
	if (!WeaponRef.Get())
	{
		WeaponRef = InWeapon;
		ATPSWeaponBase* pWeapon = WeaponRef.Get();
		if (ensure(pWeapon))
		{
			if (!bInfiniteAmmo && !pWeapon->HasAmmo()) return;
		}
	}

	// ② 상태 전환 + 델리게이트 브로드캐스트
	ViewPointGetter = InViewPointGetter;

	bIsFiring = true;

	OnFireStateChangedDelegate.Broadcast(true);

	// ③ 즉시 1발 발사 + 연사 타이머 등록 (관통탄이면 연사력 하락)
	FireOnce();

	const float BaseInterval = InWeapon->GetFireInterval();
	const float FinalInterval = bPenetration ? BaseInterval * PenetrationFireIntervalMultiplier : BaseInterval;

	GetWorld()->GetTimerManager().SetTimer(
		FireTimerHandle, this, &UTPSFireComponent::FireOnce,
		FinalInterval, true);

	UE_LOG(FireLog, Log, TEXT("[StartFire] Firing started. FireInterval: %.3f (Penetration: %s)"),
		FinalInterval, bPenetration ? TEXT("ON") : TEXT("OFF"));
}

void UTPSFireComponent::StopFire()
{
	if (!bIsFiring) return;

	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	bIsFiring = false;

	WeaponRef.Reset();
	OnFireStateChangedDelegate.Broadcast(false);

	UE_LOG(FireLog, Log, TEXT("[StopFire] Firing stopped."));
}

void UTPSFireComponent::FireOnce()
{
	// ① 무기 유효성 + 탄약 확인
	ATPSWeaponBase* pWeapon = WeaponRef.Get();
	if (!ensure(pWeapon))
	{
		StopFire();
		return;
	}

	if (!bInfiniteAmmo && !pWeapon->HasAmmo())
	{
		UE_LOG(FireLog, Log, TEXT("[FireOnce] Out of ammo. Stopping fire."));
		StopFire();
		return;
	}

	// ② 탄약 소모 + 총구 정보 획득 (무한탄창이면 스킵)
	if (!bInfiniteAmmo)
	{
		pWeapon->ConsumeAmmo();
	}

	const FTransform MuzzleTransform = pWeapon->GetMuzzleTransform();

	// ③ 카메라 중심 → 충돌 지점 방향 계산
	const FVector ShotDirection = CalculateShotDirection(MuzzleTransform.GetLocation());

	// ④ 총구 이펙트 + 풀에서 투사체 활성화
	SpawnMuzzleEffect(MuzzleTransform);
	ActivateProjectileFromPool(MuzzleTransform, ShotDirection, pWeapon);

	// ⑤ 발사 1회 델리게이트 → AnimInstance 몽타주 재생
	OnFireOnceDelegate.ExecuteIfBound();
}

FVector UTPSFireComponent::CalculateShotDirection(const FVector& InMuzzleLocation) const
{
	// ① 카메라 뷰포인트 획득
	FVector ViewLocation;
	FRotator ViewRotation;

	ViewPointGetter(ViewLocation, ViewRotation);

	// ② 카메라 → 전방 라인 트레이스
	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * 10000.f;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(WeaponRef.Get());

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	// ③ 충돌 지점(또는 최대 거리) → 총구 기준 방향 반환
	const FVector TargetPoint = bHit ? HitResult.ImpactPoint : TraceEnd;
	return (TargetPoint - InMuzzleLocation).GetSafeNormal();
}

void UTPSFireComponent::SpawnMuzzleEffect(const FTransform& InMuzzleTransform)
{
	if (!MuzzleFlashEffectAsset) return;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(), MuzzleFlashEffectAsset,
		InMuzzleTransform.GetLocation(), InMuzzleTransform.GetRotation().Rotator(),
		FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
}

void UTPSFireComponent::StartReload(ATPSWeaponBase* InWeapon)
{
	if (bIsReloading) return;
	if (bIsFiring) return;
	if (!ensure(InWeapon)) return;

	// ① 탄창이 가득이면 무시
	if (InWeapon->IsAmmoFull())
	{
		UE_LOG(FireLog, Log, TEXT("[StartReload] Ammo full. Ignoring."));
		return;
	}

	// ② 무기 참조 캐싱 + 상태 전환
	WeaponRef = InWeapon;
	bIsReloading = true;

	// ③ 델리게이트 브로드캐스트 → SoldierBase(상태) + AnimInstance(몽타주)
	OnReloadStateChangedDelegate.Broadcast(true);
	OnReloadMontagePlayDelegate.Broadcast();

	UE_LOG(FireLog, Log, TEXT("[StartReload] Reload started. ReloadTime: %.2f"), InWeapon->GetReloadTime());
}

void UTPSFireComponent::CancelReload()
{
	if (!bIsReloading) return;

	bIsReloading = false;
	WeaponRef.Reset();

	OnReloadStateChangedDelegate.Broadcast(false);

	UE_LOG(FireLog, Warning, TEXT("[CancelReload] Reload cancelled."));
}

void UTPSFireComponent::OnReloadNotify()
{
	if (!bIsReloading) return;

	// AnimNotify 시점: 실제 탄약 충전
	ATPSWeaponBase* pWeapon = WeaponRef.Get();
	if (ensure(pWeapon))
	{
		pWeapon->ReloadAmmo();
		UE_LOG(FireLog, Log, TEXT("[OnReloadNotify] Ammo replenished: %d/%d"),
			pWeapon->GetCurrentAmmo(), pWeapon->GetMaxAmmo());
	}
}

void UTPSFireComponent::OnReloadMontageFinished(bool bInterrupted)
{
	if (!bIsReloading) return;

	bIsReloading = false;
	WeaponRef.Reset();

	if (bInterrupted)
	{
		UE_LOG(FireLog, Warning, TEXT("[OnReloadMontageFinished] Reload montage interrupted."));
	}
	else
	{
		UE_LOG(FireLog, Log, TEXT("[OnReloadMontageFinished] Reload complete."));
	}

	OnReloadStateChangedDelegate.Broadcast(false);
}

void UTPSFireComponent::SetPenetration(bool bInValue)
{
	if (bPenetration == bInValue) return;
	bPenetration = bInValue;

	// 사격 중이면 연사 타이머를 새 간격으로 재등록
	if (bIsFiring)
	{
		ATPSWeaponBase* pWeapon = WeaponRef.Get();
		if (pWeapon)
		{
			const float BaseInterval = pWeapon->GetFireInterval();
			const float FinalInterval = bPenetration ? BaseInterval * PenetrationFireIntervalMultiplier : BaseInterval;

			GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				FireTimerHandle, this, &UTPSFireComponent::FireOnce,
				FinalInterval, true);

			UE_LOG(FireLog, Log, TEXT("[SetPenetration] Timer updated mid-fire. NewInterval: %.3f"), FinalInterval);
		}
	}
}

void UTPSFireComponent::ActivateProjectileFromPool(const FTransform& InMuzzleTransform, const FVector& InDirection, ATPSWeaponBase* InWeapon)
{
	// ① 풀 서브시스템에서 투사체 꺼내기
	UTPSProjectilePoolSubsystem* pPool = GetWorld()->GetSubsystem<UTPSProjectilePoolSubsystem>();
	if (!ensure(pPool)) return;

	ATPSProjectileBase* pProjectile = bPenetration
		? pPool->GetPenetratingProjectile()
		: pPool->GetProjectile();
	if (!ensure(pProjectile)) return;

	// ② Instigator/Owner 설정 + 투사체 활성화
	pProjectile->SetInstigator(Cast<APawn>(GetOwner()));
	pProjectile->SetOwner(GetOwner());
	pProjectile->ActivateProjectile(InMuzzleTransform, InDirection, InWeapon->GetDamage(), InWeapon->GetProjectileSpeed());
}