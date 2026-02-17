#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSFireComponent.generated.h"

/** 사격 상태 변경 시 브로드캐스트 (bIsFiring: true=사격 시작, false=사격 종료) */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFireStateChanged, bool /* bIsFiring */);

/**
 * 사격 관리 컴포넌트
 * - StartFire: 타이머 기반 연사 시작 (무기의 FireRate 사용)
 * - FireOnce: 뷰포인트 → 머즐 방향 계산 → 풀에서 발사체 활성화
 * - 머즐 이펙트 (Niagara) 스폰
 * - ProjectilePoolSubsystem에서 발사체 Get/Return
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSFireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * 사격 시작
	 *
	 * @param InWeapon           현재 장착 무기
	 * @param InViewPointGetter  카메라 뷰포인트 획득 함수 (Controller에서 바인딩)
	 */
	void StartFire(class ATPSWeaponBase* InWeapon, TFunction<void (FVector&, FRotator&)> InViewPointGetter);

	/** 사격 종료 (타이머 해제) */
	void StopFire();

	FORCEINLINE bool GetIsFiring() const { return bIsFiring; }

	FOnFireStateChanged OnFireStateChangedDelegate;

protected:
	/** 1발 발사 (타이머 콜백) */
	void FireOnce();

	/** 뷰포인트 → 머즐 위치 기준 사격 방향 계산 */
	FVector CalculateShotDirection(const FVector& InMuzzleLocation) const;

	/** 머즐 플래시 Niagara 이펙트 스폰 */
	void SpawnMuzzleEffect(const FTransform& InMuzzleTransform);

	/** 풀에서 발사체 꺼내 활성화 */
	void ActivateProjectileFromPool(const FTransform& InMuzzleTransform, const FVector& InDirection, class ATPSWeaponBase* InWeapon);

	/** 현재 사격 중인 무기 (WeakPtr — Tick 외) */
	TWeakObjectPtr<class ATPSWeaponBase> WeaponRef;

	/** 연사 타이머 핸들 */
	FTimerHandle FireTimerHandle;

	uint8 bIsFiring : 1 = false;

	/** 카메라 뷰포인트 획득 콜백 */
	TFunction<void (FVector&, FRotator&)> ViewPointGetter;

	/** 머즐 플래시 이펙트 에셋 */
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<class UNiagaraSystem> MuzzleFlashEffectAsset;
};