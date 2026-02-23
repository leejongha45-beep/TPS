#pragma once

#include <entt/entt.hpp>

/**
 * 이동 시스템 — CTransform + CMovementPrev를 가진 Entity 순회
 *
 * [GameThread] EnemyScheduler::Tick()에서 호출
 * - Read:  CMovementPrev.Velocity → Cached 지역변수
 * - Write: Cached값 파라미터로 CTransform.Position 갱신
 * - PushToPrev: 갱신된 CTransform → CTransformPrev
 */
namespace MovementSystem
{
	void Tick(entt::registry& Registry, float DeltaTime);
};
