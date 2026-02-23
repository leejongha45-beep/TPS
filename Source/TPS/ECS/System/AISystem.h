#pragma once

#include <entt/entt.hpp>
#include "Math/Vector.h"

/**
 * AI 시스템 — 적 상태 결정 + 이동 방향 설정
 *
 * [GameThread→Worker] EnemyScheduler::Tick()에서 호출, 내부 ParallelFor
 * - Read:  CTransformPrev.Position, CHealthPrev.Current, CMovementPrev.MaxSpeed
 * - Write: CEnemyState.State, CMovement.Velocity
 * - PushToPrev: CEnemyState → CEnemyStatePrev, CMovement → CMovementPrev
 *
 * 스레드 안전성:
 * - 각 Entity 컴포넌트는 독립 메모리 → Entity별 병렬 처리 안전
 * - Dying/Dead 스킵 → DeathSystem Write 대상과 상호 배타
 */
namespace AISystem
{
	void Tick(entt::registry& Registry, float DeltaTime, const FVector& PlayerPosition, float AttackRange);
};
