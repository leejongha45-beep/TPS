#pragma once


#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

/**
 * 사망 마킹 시스템 — Dying + 사망 애니메이션 완료 시 Dead 전환
 *
 * [GameThread→Worker] EnemyScheduler::Tick()에서 호출, 내부 ParallelFor
 * - Read:  CEnemyStatePrev.State == Dying, CAnimationPrev.AnimTime >= Duration
 * - Write: CEnemyState.State = Dead
 * - PushToPrev: CEnemyState → CEnemyStatePrev
 *
 * 스레드 안전성:
 * - Dying Entity만 처리 → AISystem Write 대상(비-Dying)과 상호 배타
 * - Phase 4에서 실행 → Animation/Movement(Phase 5+6)와 시간적 분리
 */
namespace DeathSystem
{
	void Tick(entt::registry& Registry);
};