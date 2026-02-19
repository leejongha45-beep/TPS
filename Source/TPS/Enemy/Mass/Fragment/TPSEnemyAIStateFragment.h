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
};
