#pragma once

#include <entt/entt.hpp>
#include "Math/Vector.h"

/**
 * AI 시스템 — 적 상태 결정 + 이동 방향 설정
 *
 * [GameThread] EnemyScheduler::Tick()에서 호출
 * - Read:  CTransformPrev.Position, CHealthPrev.Current, CMovementPrev.MaxSpeed
 * - Write: CEnemyState.State, CMovement.Velocity
 * - PushToPrev: CEnemyState → CEnemyStatePrev, CMovement → CMovementPrev
 */
namespace AISystem
{
	void Tick(entt::registry& Registry, float DeltaTime, const FVector& PlayerPosition, float AttackRange);
};
