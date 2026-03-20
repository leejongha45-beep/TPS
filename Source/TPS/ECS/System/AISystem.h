#pragma once

#include "Containers/Array.h"
#include "Math/Vector.h"
#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

/**
 * AI 시스템 — 적 상태 결정 + 이동 방향 설정
 *
 * [GameThread→Worker] EnemyScheduler::Tick()에서 호출, 내부 ParallelFor
 * - Read:  CTransformPrev, CHealthPrev, CMovementPrev, CLODPrev, CAIModePrev, CNavTargetPrev, CWaypointPrev
 * - Write: CEnemyState, CMovement, CAIMode, CWaypoint
 * - PushToPrev: CEnemyState, CMovement, CWaypoint
 *
 * Rush 모드: CachedWaypoints[CurrentIndex] → 웨이포인트 방향 이동
 * Chase 모드: CNavTargetPrev.NextWaypoint → 플레이어 방향 이동
 * 전환: 피격(HP감소) OR 플레이어 탐지 범위(AggroRadius) → Chase
 */
namespace AISystem
{
	void Tick(entt::registry& Registry, const FVector& PlayerPosition,
	          float AttackRange, const TArray<FVector>& Waypoints,
	          float WaypointAcceptRadius,
	          const TArray<FVector>& NPCPositions);
};
