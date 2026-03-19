#include "ECS/System/MovementSystem.h"
#include "ECS/Component/Components.h"
#include "ECS/System/FlowFieldSystem.h"
#include "Async/ParallelFor.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"


void MovementSystem::UpdateChaseTargets(entt::registry& Registry, UWorld* World,
                                        const FVector& PlayerPosition, int32 FrameCounter)
{
	if (FrameCounter % ECSConstants::NavPathRefreshInterval != 0) { return; }

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	auto View = Registry.view<CAIModePrev, CTransformPrev, CNavTarget, CNavTargetPrev>();

	for (auto Entity : View)
	{
		// ① Read — Prev → 지역변수
		const EAIMode CachedMode = View.get<CAIModePrev>(Entity).Mode;
		if (CachedMode != EAIMode::Chase) { continue; }
		const FVector CachedPosition = View.get<CTransformPrev>(Entity).Position;

		// ② Write — 지역변수로 계산 → Current에 쓰기
		FVector NewWaypoint = PlayerPosition;
		if (NavSys)
		{
			UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(
				World, CachedPosition, PlayerPosition);
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

void MovementSystem::Tick(entt::registry& Registry, float DeltaTime, const FFlowField& FlowField)
{
	auto View = Registry.view<CTransform, CTransformPrev, CMovementPrev, CEnemyStatePrev, CLODPrev>();

	// ── Entity 수집 ──
	TArray<entt::entity, TInlineAllocator<3000>> Entities;
	Entities.Reserve(View.size_hint());
	for (auto Entity : View) { Entities.Add(Entity); }

	const int32 Count = Entities.Num();

	// ── ParallelFor: Entity별 독립 처리 ── [WorkerThread]
	ParallelFor(Count, [&View, &Entities, &FlowField](int32 Index)
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
		const float GroundZ = FlowField.LookupHeight(NewPosition.X, NewPosition.Y);
		if (GroundZ > FFlowField::InvalidHeight) { NewPosition.Z = GroundZ; }

		// ③ Write — Current에 쓰기
		View.get<CTransform>(Entity).Position = NewPosition;

		// ④ PushToPrev
		View.get<CTransformPrev>(Entity).Position = NewPosition;
	});
}
