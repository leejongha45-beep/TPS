#include "ECS/System/MovementSystem.h"
#include "ECS/Component/Components.h"
#include "Async/ParallelFor.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"


void MovementSystem::UpdateNavTargets(entt::registry& Registry, UWorld* World,
                                      const FVector& PlayerPosition, int32 FrameCounter,
                                      const TArray<FVector>& Waypoints,
                                      const TArray<FVector>& NPCPositions)
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
				// 모든 웨이포인트 통과 → 마지막 웨이포인트에서 정지
				GoalPosition = (WaypointCount > 0) ? Waypoints.Last() : CachedPosition;
			}
		}
		else // Chase — 플레이어 + NPC 중 가장 가까운 타겟
		{
			GoalPosition = PlayerPosition;
			float BestDistSq = FVector::DistSquared(CachedPosition, PlayerPosition);

			for (const FVector& NPCPos : NPCPositions)
			{
				const float DistSq = FVector::DistSquared(CachedPosition, NPCPos);
				if (DistSq < BestDistSq)
				{
					BestDistSq = DistSq;
					GoalPosition = NPCPos;
				}
			}
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
		// 현재 위치에서 NavMesh 수직 투영 → 지면 Z 캐싱
		float GroundZ = CachedPosition.Z;
		if (NavSys)
		{
			FNavLocation ProjectedLocation;
			const FVector ProjectionExtent(100.f, 100.f, 5000.f);
			if (NavSys->ProjectPointToNavigation(CachedPosition, ProjectedLocation, ProjectionExtent))
			{
				GroundZ = ProjectedLocation.Location.Z;
			}
		}

		auto& NavTarget = View.get<CNavTarget>(Entity);
		NavTarget.NextWaypoint = NewWaypoint;
		NavTarget.GroundZ = GroundZ;

		// ③ PushToPrev — Current → Prev
		auto& NavTargetPrev = View.get<CNavTargetPrev>(Entity);
		NavTargetPrev.NextWaypoint = NewWaypoint;
		NavTargetPrev.GroundZ = GroundZ;
	}
}

void MovementSystem::Tick(entt::registry& Registry, float DeltaTime)
{
	auto View = Registry.view<CTransform, CTransformPrev, CMovementPrev, CEnemyStatePrev, CLODPrev, CNavTargetPrev>();

	// ── Entity 수집 ──
	TArray<entt::entity, TInlineAllocator<3000>> Entities;
	Entities.Reserve(View.size_hint());
	for (auto Entity : View) { Entities.Add(Entity); }

	const int32 Count = Entities.Num();

	// ── ParallelFor: Entity별 독립 처리 ── [WorkerThread]
	ParallelFor(Count, [&View, &Entities](int32 Index)
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
		const FQuat CachedRotation = View.get<CTransformPrev>(Entity).Rotation;
		const float CachedGroundZ = View.get<CNavTargetPrev>(Entity).GroundZ;

		// ② 계산 — 지역변수만 사용
		FVector NewPosition = CachedPosition + CachedVelocity * CachedAccumDT;

		// Z 보정 — NavMesh 투영 지면 높이로 보간
		constexpr float ZInterpSpeed = 5.f;
		NewPosition.Z = FMath::FInterpTo(CachedPosition.Z, CachedGroundZ, CachedAccumDT, ZInterpSpeed);

		// 회전 — 실제 이동 방향(Z 포함)으로 보간
		FQuat NewRotation = CachedRotation;
		const FVector MoveDir = NewPosition - CachedPosition;
		if (!MoveDir.IsNearlyZero(1.f))
		{
			constexpr float RotInterpSpeed = 8.f;
			const FQuat TargetQuat = MoveDir.ToOrientationQuat();
			const float Alpha = FMath::Clamp(RotInterpSpeed * CachedAccumDT, 0.f, 1.f);
			NewRotation = FQuat::Slerp(CachedRotation, TargetQuat, Alpha);
		}

		// ③ Write — Current에 쓰기
		View.get<CTransform>(Entity).Position = NewPosition;
		View.get<CTransform>(Entity).Rotation = NewRotation;

		// ④ PushToPrev
		View.get<CTransformPrev>(Entity).Position = NewPosition;
		View.get<CTransformPrev>(Entity).Rotation = NewRotation;
	});
}
