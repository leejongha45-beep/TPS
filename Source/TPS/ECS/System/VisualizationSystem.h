#pragma once

#include "CoreTypes.h"
#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

class UInstancedStaticMeshComponent;

/**
 * 시각화 시스템 — ECS → HISM 동기화 (LOD별 호출)
 *
 * [GameThread] 매 프레임 LOD별 Tick
 * - LODLevel 필터: 해당 LOD에 속한 Entity만 처리
 * - 변경 감지: Transform/Animation 분리 비교
 */
namespace VisualizationSystem
{
	void Tick(entt::registry& Registry,
	          class UInstancedStaticMeshComponent* HISM,
	          uint8 LODLevel);
};