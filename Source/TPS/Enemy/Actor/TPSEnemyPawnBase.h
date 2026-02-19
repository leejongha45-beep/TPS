#pragma once
#include "CoreMinimal.h"
#include "MassEntityHandle.h"
#include "GameFramework/Pawn.h"
#include "Utils/Interface/Action/AIControllable.h"
#include "Utils/TickFunctions/FAIControlTickFunction.h"
#include "Utils/UENUM/EnemyState.h"
#include "MassEntityTypes.h"
#include "TPSEnemyPawnBase.generated.h"

/**
 * 경량 적 폰 베이스
 * - APawn 상속 (ACharacter 아님 — CMC 오버헤드 회피)
 * - IAIControllable 구현 — FAIControlTickFunction으로 AI Tick
 * - CapsuleComponent: 데미지 판정용 충돌
 * - SkeletalMeshComponent: 애니메이션 렌더링
 * - HealthComponent: TakeDamage 수신 → Mass Fragment 동기화
 * - 풀링 지원: Activate/Deactivate 패턴
 *
 * 추후 경직/넉백 처리는 FOnEnemyDamaged 델리게이트 수신으로 구현 예정
 */
UCLASS(Abstract)
class TPS_API ATPSEnemyPawnBase : public APawn, public IAIControllable
{
	GENERATED_BODY()

public:
	ATPSEnemyPawnBase();

	/** 풀에서 꺼내 활성화 — 위치/회전 설정 + AI Tick 시작 */
	void ActivateEnemy(const FTransform& InTransform, EEnemyAIState InInitialState = EEnemyAIState::Chase);

	/** 비활성화 후 풀에 반환 — AI Tick 정지 */
	void DeactivateEnemy();

	/** 사망 연출 시작 (충돌OFF → 타이머 → DeactivateEnemy) */
	void PlayDeath();

	FORCEINLINE class UTPSEnemyHealthComponent* GetHealthComponent() const { return HealthComponentInst; }

	/** Mass Entity Handle 설정 — LODProcessor가 Actor 스폰 시 호출 */
	void SetMassEntityHandle(FMassEntityHandle InHandle, FMassEntityManager* InEntityManager);
	FORCEINLINE FMassEntityHandle GetMassEntityHandle() const { return MassEntityHandle; }

protected:
	virtual void PostInitializeComponents() override;
	virtual void RegisterActorTickFunctions(bool bRegister) override;

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

#pragma region AIControl
	/** IAIControllable — AI 제어 Tick */
	virtual void AI_Control_Tick(float DeltaTime) override;

	/** AI Tick Enable/Disable 헬퍼 */
	void SetAIControlTickEnabled(bool bEnabled);

	/** AI 제어 전용 Tick 함수 */
	FAIControlTickFunction AIControlTickFunction;

	/** 상태별 처리 함수 */
	void ProcessIdle(float DeltaTime);
	void ProcessChase(float DeltaTime, APawn* InTarget, float InDistance);
	void ProcessAttack(float DeltaTime, APawn* InTarget, float InDistance);

	/** 사망 델리게이트 수신 → PlayDeath 호출 */
	void OnDeath();

	/** 현재 AI 상태 */
	EEnemyAIState CurrentAIState = EEnemyAIState::Idle;

	/** Idle 상태 경과 시간 */
	float StateTimer = 0.f;

	/** 공격 쿨다운 잔여 */
	float AttackCooldownTimer = 0.f;

	/** 이동 속도 (cm/s) */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float MoveSpeed = 600.f;

	/** 공격 사거리 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AttackRange = 150.f;

	/** 공격 데미지 */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AttackDamage = 10.f;

	/** 공격 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AttackInterval = 1.f;

	/** Idle 대기 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float IdleWaitTime = 0.5f;
#pragma endregion

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