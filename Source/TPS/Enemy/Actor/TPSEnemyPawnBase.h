#pragma once
#include "CoreMinimal.h"
#include "MassEntityHandle.h"
#include "GameFramework/Pawn.h"
#include "Utils/UENUM/EnemyState.h"
#include "MassEntityTypes.h"
#include "TPSEnemyPawnBase.generated.h"

/**
 * 경량 적 폰 베이스 — 시각/충돌 껍데기
 * - APawn 상속 (ACharacter 아님 — CMC 오버헤드 회피)
 * - CapsuleComponent: 데미지 판정용 충돌
 * - SkeletalMeshComponent: 애니메이션 렌더링
 * - HealthComponent: TakeDamage 수신 → Mass Fragment 동기화
 * - 풀링 지원: Activate/Deactivate 패턴
 *
 * 모든 AI 로직은 Processor가 권위자:
 *   상태 전환 → AIProcessor, 이동 → MovementProcessor+SyncProcessor, 데미지 → MeleeProcessor
 * Actor는 SyncProcessor에서 상태/위치를 수신하여 시각적 표현만 담당
 */
UCLASS(Abstract)
class TPS_API ATPSEnemyPawnBase : public APawn
{
	GENERATED_BODY()

public:
	ATPSEnemyPawnBase();

	/** 풀에서 꺼내 활성화 — 위치/회전 설정 */
	void ActivateEnemy(const FTransform& InTransform, EEnemyAIState InInitialState = EEnemyAIState::Chase);

	/** 비활성화 후 풀에 반환 */
	void DeactivateEnemy();

	/** 사망 연출 시작 (충돌OFF → 타이머 → DeactivateEnemy) */
	void PlayDeath();

	FORCEINLINE class UTPSEnemyHealthComponent* GetHealthComponent() const { return HealthComponentInst; }

	/** Mass Entity Handle 설정 — LODProcessor가 Actor 스폰 시 호출 */
	void SetMassEntityHandle(FMassEntityHandle InHandle, FMassEntityManager* InEntityManager);
	FORCEINLINE FMassEntityHandle GetMassEntityHandle() const { return MassEntityHandle; }

	/** SyncProcessor가 호출 — Fragment AI 상태를 Actor에 반영 (시각/사운드 전환) */
	void SyncAIState(EEnemyAIState InNewState);

	/** 현재 동기화된 AI 상태 (읽기 전용) */
	FORCEINLINE EEnemyAIState GetSyncedAIState() const { return SyncedAIState; }

protected:
	virtual void PostInitializeComponents() override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

#pragma region Component
	/** 충돌 캡슐 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	TObjectPtr<class UCapsuleComponent> CapsuleComponentInst;

	/** 스켈레탈 메시 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	TObjectPtr<class USkeletalMeshComponent> MeshComponentInst;

	/** 체력 관리 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	TObjectPtr<class UTPSEnemyHealthComponent> HealthComponentInst;
#pragma endregion

	/** 사망 델리게이트 수신 → PlayDeath 호출 */
	void OnDeath();

	/** Fragment에서 동기화된 현재 AI 상태 (AIProcessor가 권위자) */
	EEnemyAIState SyncedAIState = EEnemyAIState::Idle;

	/** 사망 후 비활성화까지 대기 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathLingerTime = 2.f;

	FTimerHandle DeathTimerHandle;

#pragma region MassEntity
	/** 대응되는 Mass Entity (LODProcessor가 설정) */
	FMassEntityHandle MassEntityHandle;

	/** Entity Fragment 접근용 */
	FMassEntityManager* CachedEntityManager = nullptr;
#pragma endregion
};
