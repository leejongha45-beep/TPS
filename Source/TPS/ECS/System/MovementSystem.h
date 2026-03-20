#pragma once

#include "Containers/Array.h"
#include "Math/Vector.h"
#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

struct FTerrainHeightCache;

/**
 * 이동 시스템 — CTransform 갱신 + Z값 지형 보정
 *
 * UpdateNavTargets: [GameThread] Phase 1.1
 * - Rush/Chase 엔티티 NavMesh 경로 쿼리 + CNavTarget PushToPrev
 * - Rush:  Waypoints[CurrentIndex] 방향 NavMesh 경로
 * - Chase: PlayerPosition 방향 NavMesh 경로
 * - Read:  CAIModePrev, CTransformPrev, CWaypointPrev
 * - Write: CNavTarget
 * - PushToPrev: CNavTarget → CNavTargetPrev
 *
 * Tick: [WorkerThread] Phase 6
 * - 내부 ParallelFor로 Entity별 병렬 처리
 * - Read:  CMovementPrev.Velocity, CEnemyStatePrev, CLODPrev, FTerrainHeightCache.Heights
 * - Write: CTransform.Position
 * - PushToPrev: CTransform → CTransformPrev
 */
namespace MovementSystem
{
	void UpdateNavTargets(entt::registry& Registry, class UWorld* InWorld,
	                      const FVector& PlayerPosition, int32 FrameCounter,
	                      const TArray<FVector>& Waypoints, const FVector& BaseLocation);
	void Tick(entt::registry& Registry, float DeltaTime, const FTerrainHeightCache& TerrainCache);
};
