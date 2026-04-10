#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/TPSProjectileBase.h"
#include "TPSPenetratingProjectile.generated.h"

/**
 * 관통 발사체 — PsychoSync Phase 2에서 사용
 * - Enemy 채널 Overlap → 적을 물리적으로 뚫고 지나감 (OnBeginOverlap으로 데미지)
 * - 벽/지형 Block → OnComponentHit → 비활성화
 */
UCLASS()
class TPS_API ATPSPenetratingProjectile : public ATPSProjectileBase
{
	GENERATED_BODY()

public:
	ATPSPenetratingProjectile();

	virtual void ActivateProjectile(const FTransform& InMuzzleTransform, const FVector& InDirection, float InDamage, float InSpeed) override;

protected:
	/** 벽/지형 Block 충돌 시 — 비활성화만 */
	virtual void HandleHit(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, const FHitResult& Hit) override;

	/** 적 Overlap 충돌 시 — 데미지 적용 + 관통 */
	UFUNCTION()
	void OnOverlapEnemy(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
		class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	/** 최대 관통 횟수 (0 = 무제한) */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Penetration")
	int32 MaxPenetrationCount = 0;

	int32 CurrentPenetrationCount = 0;

	/** 이미 데미지를 적용한 HISM 인스턴스 인덱스 (중복 방지) */
	TSet<int32> HitInstanceIndices;

	/** 이미 데미지를 적용한 액터 (중복 방지) */
	TSet<TWeakObjectPtr<AActor>> HitActors;

	/** 관통탄 전용 트레일 이펙트 (스크류 궤적) */
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<class UNiagaraSystem> PenetratingTrailAsset;
};