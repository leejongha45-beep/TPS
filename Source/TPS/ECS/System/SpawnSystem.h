#pragma once

#include <entt/entt.hpp>
#include "ECS/Data/EnemySpawnParams.h"

/**
 * 스폰 시스템 — Entity 생성 + 모든 컴포넌트(Current/Prev 쌍) emplace
 *
 * [GameThread] 호출 시점에 1회 실행 (Tick 기반 아님)
 * - Current와 Prev를 동일 초기값으로 emplace (첫 프레임 Read 유효)
 */
namespace SpawnSystem
{
	entt::entity Spawn(entt::registry& Registry, const FEnemySpawnParams& Params);
};
