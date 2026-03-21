#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSProjectileBase.generated.h"

/**
 * 발사체 베이스 클래스
 * - ProjectilePoolSubsystem에서 풀링 관리
 * - Activate: 풀에서 꺼내 위치/방향/속도 설정 후 활성화
 * - Deactivate: 충돌 또는 수명 만료 시 풀로 반환
 * - OnHit: 충돌 시 데미지 적용 + 임팩트 이펙트
 */
UCLASS()
class TPS_API ATPSProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	ATPSProjectileBase();

	/**
	 * 풀에서 꺼내 활성화
	 *
	 * @param InMuzzleTransform  머즐 위치/회전
	 * @param InDirection        사격 방향
	 * @param InDamage           데미지
	 * @param InSpeed            발사체 속도
	 */
	virtual void ActivateProjectile(const FTransform& InMuzzleTransform, const FVector& InDirection, float InDamage, float InSpeed);

	/** 비활성화 후 풀에 반환 */
	void DeactivateProjectile();

protected:
	/** 충돌 시 콜백 — HandleHit 호출 */
	UFUNCTION()
	void OnHit(class UPrimitiveComponent* HitComponent, class AActor* OtherActor,
		class UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

	/** 충돌 처리 — 자식에서 override 가능 */
	virtual void HandleHit(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, const FHitResult& Hit);

	/** 수명 만료 시 비활성화 */
	void OnLifeSpanExpired();

	/** 충돌 구체 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Collision")
	TObjectPtr<class USphereComponent> CollisionComponentInst;

	/** 발사체 무브먼트 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Movement")
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovementInst;

	/** 충돌 임팩트 Niagara 이펙트 */
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<class UNiagaraSystem> ImpactEffectAsset;

	/** 발사체 최대 수명 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float LifeSpan = 3.f;

	float Damage = 0.f;
	FTimerHandle LifeSpanTimerHandle;
};
