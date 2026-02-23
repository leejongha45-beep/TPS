#pragma once

#include <entt/entt.hpp>

/**
 * 애니메이션 시스템 — VAT AnimTime 갱신
 *
 * [GameThread] 매 프레임 Tick
 * - CEnemyStatePrev 기반으로 AnimIndex 결정
 * - AnimTime += DeltaTime * PlayRate
 */
namespace AnimationSystem
{
	void Tick(entt::registry& Registry, float DeltaTime);
};
