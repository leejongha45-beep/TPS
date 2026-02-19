#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TPSEnemyPawnBase.generated.h"

/**
 * 경량 적 폰 베이스
 * - APawn 상속 (ACharacter 아님 — CMC 오버헤드 회피)
 * - CapsuleComponent: 데미지 판정용 충돌
 * - SkeletalMeshComponent: 애니메이션 렌더링
 * - HealthComponent: TakeDamage 수신 → Mass Fragment 동기화
 * - Mass Entity Handle 캐싱 → 데이터 레이어 양방향 동기화
 * - 풀링 지원: Activate/Deactivate 패턴
 *
 * 추후 경직/넉백 처리는 FOnEnemyDamaged 델리게이트 수신으로 구현 예정
 */
UCLASS(Abstract)
class TPS_API ATPSEnemyPawnBase : public APawn
{
	GENERATED_BODY()

public:
	ATPSEnemyPawnBase();

	/** 풀에서 꺼내 활성화 — 위치/회전 설정 + Mass Handle 바인딩 */
	void ActivateEnemy(const FTransform& InTransform);

	/** 비활성화 후 풀에 반환 */
	void DeactivateEnemy();

	/** 사망 연출 시작 (충돌OFF → 타이머 → DeactivateEnemy) */
	void PlayDeath();

	FORCEINLINE class UTPSEnemyHealthComponent* GetHealthComponent() const { return HealthComponentInst; }

protected:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	/** 충돌 캡슐 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	TObjectPtr<class UCapsuleComponent> CapsuleComponentInst;

	/** 스켈레탈 메시 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	TObjectPtr<class USkeletalMeshComponent> MeshComponentInst;

	/** 체력 관리 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	TObjectPtr<class UTPSEnemyHealthComponent> HealthComponentInst;

	/** 사망 후 비활성화까지 대기 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathLingerTime = 2.f;

	FTimerHandle DeathTimerHandle;
};
