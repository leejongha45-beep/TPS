#pragma once

#include <entt/entt.hpp>

/**
 * 사망 마킹 시스템 — Dying + 사망 애니메이션 완료 시 Dead 전환
 *
 * [GameThread] EnemyScheduler::Tick()에서 호출
 * - Read:  CEnemyStatePrev.State == Dying, CAnimationPrev.AnimTime ≥ Duration
 * - Write: CEnemyState.State = Dead
 * - PushToPrev: CEnemyState → CEnemyStatePrev
 */
namespace DeathSystem
{
	void Tick(entt::registry& Registry);
};
