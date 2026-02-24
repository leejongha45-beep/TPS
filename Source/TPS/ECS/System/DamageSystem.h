#pragma once

#include "ECS/Data/DamageEvent.h"
#include <entt/entt.hpp>

/**
 * 데미지 시스템 — 외부 데미지 큐 → CHealth 반영
 *
 * - Read:  DamageQueue (외부 입력), InstanceToEntity (O(1) 룩업)
 * - Write: CHealth.Current -= Damage
 * - PushToPrev: CHealth → CHealthPrev
 *
 * Dying/Dead Entity 스킵 (불필요한 쓰기 방지)
 * 같은 Entity 다중 히트: 큐 순차 처리 → CHealth.Current 누적 감산
 */
namespace DamageSystem
{
	void Tick(entt::registry& Registry,
	          TArray<FDamageEvent>& DamageQueue,
	          const TArray<entt::entity>& InstanceToEntity);
};
