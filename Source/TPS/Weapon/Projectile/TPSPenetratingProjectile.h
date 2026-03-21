#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/TPSProjectileBase.h"
#include "TPSPenetratingProjectile.generated.h"

/**
 * 관통 발사체 — PsychoSync Phase 2에서 사용
 * - 적 타격 시 데미지 적용 후 관통 (비활성화 안 함)
 * - 벽/지형 타격 시에는 비활성화
 * - bShouldBounce=true → StopSimulating 방지 → 바운스 콜백에서 직진 복원
 */
UCLASS()
class TPS_API ATPSPenetratingProjectile : public ATPSProjectileBase
{
	GENERATED_BODY()

public:
	ATPSPenetratingProjectile();

	virtual void ActivateProjectile(const FTransform& InMuzzleTransform, const FVector& InDirection, float InDamage, float InSpeed) override;

protected:
	/** 충돌 처리 오버라이드 — 적 관통, 벽/지형만 비활성화 */
	virtual void HandleHit(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, const FHitResult& Hit) override;

	/** 바운스 콜백 — Block 충돌 후 원래 방향으로 속도 복원 */
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	/** 최대 관통 횟수 (0 = 무제한) */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Penetration")
	int32 MaxPenetrationCount = 0;

	int32 CurrentPenetrationCount = 0;

	/** 이미 데미지를 적용한 HISM 인스턴스 인덱스 (중복 방지) */
	TSet<int32> HitInstanceIndices;

	/** 이미 데미지를 적용한 액터 (중복 방지) */
	TSet<TWeakObjectPtr<AActor>> HitActors;

	/** ActivateProjectile에서 캐싱한 발사 방향 */
	FVector CachedDirection = FVector::ForwardVector;
};
