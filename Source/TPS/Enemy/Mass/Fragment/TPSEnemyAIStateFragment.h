#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Utils/UENUM/EnemyState.h"
#include "TPSEnemyAIStateFragment.generated.h"

/**
 * 적 AI 상태 Fragment
 * - Mass Entity 데이터 레이어에서 AI 상태머신 데이터 관리
 * - AIProcessor가 매 프레임 상태 전환 처리
 * - MeleeProcessor가 쿨다운/사거리 참조하여 공격 판정
 */
USTRUCT()
struct FTPSEnemyAIStateFragment : public FMassFragment
{
	GENERATED_BODY()

	/** 현재 AI 상태 */
	EEnemyAIState AIState = EEnemyAIState::Idle;

	/** Idle 대기 경과 시간 */
	float StateTimer = 0.f;

	/** 공격 쿨다운 잔여 */
	float AttackCooldownTimer = 0.f;

	/** 공격 사거리 (cm) */
	float AttackRange = 150.f;

	/** 공격 데미지 */
	float AttackDamage = 10.f;

	/** 공격 간격 (초) */
	float AttackInterval = 1.f;

	/** Idle 대기 시간 (초) */
	float IdleWaitTime = 0.5f;

	/** 공격→추격 복귀 히스테리시스 배율 (AttackRange * 이 값 초과 시 Chase 복귀) */
	float AttackRangeHysteresis = 1.2f;

	// ──────────── 어그로 / 타겟팅 ────────────

	/** 어그로 감지 범위 (cm) — 이 거리 이내 ITargetable 탐색 */
	float AggroRange = 3000.f;

	/** 어그로 해제 범위 (cm) — 현재 타겟이 이 거리 밖이면 재탐색 */
	float AggroReleaseRange = 5000.f;

	/** 현재 추적 중인 타겟 위치 (프레임 간 보존 — 배열 변동 방어용) */
	FVector CurrentTargetLocation = FVector::ZeroVector;

	/** 현재 타겟 배열 인덱스 (매 프레임 재계산 — 보존하지 않음) */
	int32 CurrentTargetIndex = INDEX_NONE;

	/** 피격 어그로 위치 (TakeDamage 시 DamageCauser 위치 스냅샷) */
	FVector HitAggroLocation = FVector::ZeroVector;

	/** 피격 어그로 활성 여부 */
	uint8 bHasHitAggro : 1 = false;
};
