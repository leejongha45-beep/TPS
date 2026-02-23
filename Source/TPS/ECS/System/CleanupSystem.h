#pragma once

#include "CoreMinimal.h"
#include <entt/entt.hpp>

class UHierarchicalInstancedStaticMeshComponent;

/**
 * 클린업 시스템 — Dead Entity의 HISM 인스턴스 제거 + Entity 파괴
 *
 * [GameThread] EnemyScheduler::Tick()에서 Visualization 이후 호출
 * - Read:  CEnemyStatePrev.State == Dead, CRenderProxyPrev.InstanceIndex
 * - Write: CRenderProxy.InstanceIndex (swap 보정, Current만 쓰기)
 * - Destroy: Entity 제거
 * - O(1) swap 보정: InstanceToEntity 역방향 룩업 테이블 사용
 */
namespace CleanupSystem
{
	void Tick(entt::registry& Registry, UHierarchicalInstancedStaticMeshComponent* HISM, TArray<entt::entity>& InstanceToEntity);
};