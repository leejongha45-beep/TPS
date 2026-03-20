#pragma once

#include "Containers/Array.h"
#include "Math/Vector.h"
#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

/**
 * 이동 시스템 — CTransform 갱신 + NavMesh 기반 Z 보정
 *
 * UpdateNavTargets: [GameThread] Phase 1.1
 * - Rush/Chase 엔티티 NavMesh 경로 쿼리 + CNavTarget PushToPrev
 * - Rush:  Waypoints[CurrentIndex] 방향 NavMesh 경로 (마지막 웨이포인트 도달 시 정지)
 * - Chase: PlayerPosition 방향 NavMesh 경로
 * - Read:  CAIModePrev, CTransformPrev, CWaypointPrev
 * - Write: CNavTarget
 * - PushToPrev: CNavTarget → CNavTargetPrev
 *
 * Tick: [WorkerThread] Phase 6
 * - 내부 ParallelFor로 Entity별 병렬 처리
 * - CNavTargetPrev.NextWaypoint.Z 값으로 Z 보간 (NavMesh 높이 반영)
 * - Read:  CMovementPrev.Velocity, CEnemyStatePrev, CLODPrev, CNavTargetPrev
 * - Write: CTransform.Position
 * - PushToPrev: CTransform → CTransformPrev
 */
namespace MovementSystem
{
	void UpdateNavTargets(entt::registry& Registry, class UWorld* InWorld,
	                      const FVector& PlayerPosition, int32 FrameCounter,
	                      const TArray<FVector>& Waypoints,
	                      const TArray<FVector>& NPCPositions);
	void Tick(entt::registry& Registry, float DeltaTime);
};
