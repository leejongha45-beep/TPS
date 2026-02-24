#pragma once

#include <entt/entt.hpp>

class UHierarchicalInstancedStaticMeshComponent;

/**
 * 시각화 시스템 — ECS → HISM 동기화
 *
 * [GameThread] 매 프레임 Tick
 * - Read:  CTransformPrev, CAnimationPrev, CRenderProxyPrev,
 *          CLODPrev (bShouldTick), CVisCachePrev (변경 감지)
 * - Write: HISM (UpdateInstanceTransform, SetCustomDataValue),
 *          CVisCache → PushToPrev → CVisCachePrev
 *
 * 최적화:
 * - LOD 스킵: bShouldTick=false → HISM 갱신 건너뜀
 * - 변경 감지: Transform/Animation 분리 비교
 *   Transform 불변 (비이동 상태) → UpdateInstanceTransform 스킵
 *   Animation 불변 (Dead) → SetCustomDataValue 스킵
 */
namespace VisualizationSystem
{
	void Tick(entt::registry& Registry, UHierarchicalInstancedStaticMeshComponent* HISM);
};