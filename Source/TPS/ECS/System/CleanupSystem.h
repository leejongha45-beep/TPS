#pragma once

#include "CoreMinimal.h"

#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

class UInstancedStaticMeshComponent;

/**
 * 클린업 시스템 — Dead Entity의 HISM 인스턴스 제거 + Entity 파괴
 * LOD별로 호출 — 해당 LOD에 속한 Dead Entity만 처리
 */
namespace CleanupSystem
{
	int32 Tick(entt::registry& Registry,
	           class UInstancedStaticMeshComponent* HISM,
	           TArray<entt::entity>& InstanceToEntity,
	           uint8 LODLevel);
};
