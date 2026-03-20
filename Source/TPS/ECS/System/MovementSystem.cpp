#include "ECS/System/MovementSystem.h"
#include "ECS/Component/Components.h"
#include "ECS/System/FlowFieldSystem.h"
#include "Async/ParallelFor.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"


void MovementSystem::UpdateNavTargets(entt::registry& Registry, UWorld* World,
                                      const FVector& PlayerPosition, int32 FrameCounter,
                                      const TArray<FVector>& Waypoints, const FVector& BaseLocation)
{
	if (FrameCounter % ECSConstants::NavPathRefreshInterval != 0) { return; }

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	auto View = Registry.view<CAIModePrev, CTransformPrev, CWaypointPrev,
	                          CNavTarget, CNavTargetPrev>();

	const int32 WaypointCount = Waypoints.Num();

	for (auto Entity : View)
	{
		// ① Read — Prev → 지역변수
		const EAIMode CachedMode = View.get<CAIModePrev>(Entity).Mode;
		const FVector CachedPosition = View.get<CTransformPrev>(Entity).Position;

		// 목표 위치 결정
		FVector GoalPosition;
		if (CachedMode == EAIMode::Rush)
		{
			const int32 WPIndex = View.get<CWaypointPrev>(Entity).CurrentIndex;
			if (WPIndex < WaypointCount)
			{
				GoalPosition = Waypoints[WPIndex];
			}
			else
			{
				// 모든 웨이포인트 통과 → 기지 방향
				GoalPosition = BaseLocation;
			}
		}
		else // Chase
		{
			GoalPosition = PlayerPosition;
		}

		// ② Write — NavMesh 경로 쿼리 → Current에 쓰기
		FVector NewWaypoint = GoalPosition;
		if (NavSys)
		{
			UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(
				World, CachedPosition, GoalPosition);
			if (Path && Path->PathPoints.Num() > 1)
			{
				NewWaypoint = Path->PathPoints[1];
			}
		}
		View.get<CNavTarget>(Entity).NextWaypoint = NewWaypoint;

		// ③ PushToPrev — Current → Prev
		View.get<CNavTargetPrev>(Entity).NextWaypoint = NewWaypoint;
	}
}

void MovementSystem::Tick(entt::registry& Registry, float DeltaTime, const FTerrainHeightCache& TerrainCache)
{
	auto View = Registry.view<CTransform, CTransformPrev, CMovementPrev, CEnemyStatePrev, CLODPrev>();

	// ── Entity 수집 ──
	TArray<entt::entity, TInlineAllocator<3000>> Entities;
	Entities.Reserve(View.size_hint());
	for (auto Entity : View) { Entities.Add(Entity); }

	const int32 Count = Entities.Num();

	// ── ParallelFor: Entity별 독립 처리 ── [WorkerThread]
	ParallelFor(Count, [&View, &Entities, &TerrainCache](int32 Index)
	{
		const entt::entity Entity = Entities[Index];

		// ① Read — Prev → 지역변수
		const bool bCachedShouldTick = View.get<CLODPrev>(Entity).bShouldTick;
		if (!bCachedShouldTick) { return; }

		const EEnemyState CachedState = View.get<CEnemyStatePrev>(Entity).State;
		if (CachedState != EEnemyState::Moving) { return; }

		const FVector CachedVelocity = View.get<CMovementPrev>(Entity).Velocity;
		const float CachedAccumDT = View.get<CLODPrev>(Entity).AccumulatedDeltaTime;
		const FVector CachedPosition = View.get<CTransformPrev>(Entity).Position;

		// ② 계산 — 지역변수만 사용
		FVector NewPosition = CachedPosition + CachedVelocity * CachedAccumDT;
		const float GroundZ = TerrainCache.LookupHeight(NewPosition.X, NewPosition.Y);
		if (GroundZ > FTerrainHeightCache::InvalidHeight)
		{
			constexpr float InterpSpeed = 10.f;
			NewPosition.Z = FMath::FInterpTo(CachedPosition.Z, GroundZ, CachedAccumDT, InterpSpeed);
		}

		// ③ Write — Current에 쓰기
		View.get<CTransform>(Entity).Position = NewPosition;

		// ④ PushToPrev
		View.get<CTransformPrev>(Entity).Position = NewPosition;
	});
}
