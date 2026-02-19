#pragma once
#include "CoreMinimal.h"
#include "EnemyState.generated.h"

/**
 * 적 AI 행동 상태
 * - Mass Processor / Actor AI에서 상태 전환에 사용
 */
UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle   = 0,   /** 대기 (스폰 직후) */
	Chase  = 1,   /** 추적 (타겟 방향 이동) */
	Attack = 2,   /** 공격 (사거리 내) */
	Die    = 3,   /** 사망 (체력 0) */
};
