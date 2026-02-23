#pragma once

#include <entt/entt.hpp>

class UHierarchicalInstancedStaticMeshComponent;

/**
 * 시각화 시스템 — ECS → HISM 동기화
 *
 * [GameThread] 매 프레임 Tick
 * - CTransformPrev → UpdateInstanceTransform
 * - CAnimationPrev → SetCustomDataValue (AnimIndex, AnimTime)
 * - CRenderProxyPrev → InstanceIndex로 HISM 접근
 */
namespace VisualizationSystem
{
	void Tick(entt::registry& Registry, UHierarchicalInstancedStaticMeshComponent* HISM);
};
