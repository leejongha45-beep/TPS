#pragma once

#include "ECS/Data/DamageEvent.h"
#include "ECS/Renderer/AEnemyRenderActor.h"

#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

/**
 * 데미지 시스템 — 외부 데미지 큐 → CHealth 반영
 *
 * - Read:  DamageQueue (외부 입력), InstanceToEntityPerLOD (LOD별 O(1) 룩업)
 * - Write: CHealth.Current -= Damage
 * - PushToPrev: CHealth → CHealthPrev
 */
namespace DamageSystem
{
	void Tick(entt::registry& Registry,
	          TArray<FDamageEvent>& DamageQueue,
	          TArray<entt::entity> (&InstanceToEntityPerLOD)[HISM_LOD_COUNT]);
};
