#include "ECS/System/MovementSystem.h"
#include "ECS/Component/Components.h"
#include "ECS/System/FlowFieldSystem.h"
#include "Async/ParallelFor.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

namespace
{

/** ② Write: 캐싱된 읽기값을 파라미터로 받아 Current에만 쓰기 */
void Write(CTransform& OutTransform, float DeltaTime, const FVector& CachedVelocity)
{
	OutTransform.Position += CachedVelocity * DeltaTime;
}

/** ③ PushToPrev: 갱신된 Current → Prev 복사 (다음 프레임용) */
void PushToPrev(CTransformPrev& OutPrev, const CTransform& InCurrent)
{
	OutPrev.Position = InCurrent.Position;
	OutPrev.Rotation = InCurrent.Rotation;
}

} // anonymous namespace

void MovementSystem::UpdateChaseTargets(entt::registry& Registry, UWorld* World,
                                        const FVector& PlayerPosition, int32 FrameCounter)
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	auto ChaseView = Registry.view<CAIModePrev, CTransformPrev, CNavTarget>();

	for (auto Entity : ChaseView)
	{
		if (ChaseView.get<CAIModePrev>(Entity).Mode != EAIMode::Chase) { continue; }

		// N프레임마다 경로 갱신
		if (FrameCounter % ECSConstants::NavPathRefreshInterval != 0) { continue; }

		const FVector& EnemyPos = ChaseView.get<CTransformPrev>(Entity).Position;

		if (NavSys)
		{
			UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(
				World, EnemyPos, PlayerPosition);

			if (Path && Path->PathPoints.Num() > 1)
			{
				ChaseView.get<CNavTarget>(Entity).NextWaypoint = Path->PathPoints[1];
			}
			else
			{
				ChaseView.get<CNavTarget>(Entity).NextWaypoint = PlayerPosition;
			}
		}
		else
		{
			ChaseView.get<CNavTarget>(Entity).NextWaypoint = PlayerPosition;
		}
	}

	// PushToPrev: CNavTarget → CNavTargetPrev
	auto NavPrevView = Registry.view<CNavTarget, CNavTargetPrev>();
	for (auto Entity : NavPrevView)
	{
		NavPrevView.get<CNavTargetPrev>(Entity).NextWaypoint =
			NavPrevView.get<CNavTarget>(Entity).NextWaypoint;
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
	ParallelFor(Count, [&](int32 Index)
	{
		const entt::entity Entity = Entities[Index];

		// ① Read
		const CLODPrev& CachedLOD = View.get<CLODPrev>(Entity);
		if (!CachedLOD.bShouldTick) { return; }

		const EEnemyState CachedState = View.get<CEnemyStatePrev>(Entity).State;
		if (CachedState != EEnemyState::Moving) { return; }

		const FVector CachedVelocity = View.get<CMovementPrev>(Entity).Velocity;
		const float CachedAccumDT = CachedLOD.AccumulatedDeltaTime;

		// ② Write (AccumDT로 스킵 프레임 시간 보상)
		Write(View.get<CTransform>(Entity), CachedAccumDT, CachedVelocity);

		// ② -1. Z값 지형 보정 — FlowField Heights 캐시 참조
		CTransform& Transform = View.get<CTransform>(Entity);
		const float GroundZ = FlowField.LookupHeight(Transform.Position.X, Transform.Position.Y);
		if (GroundZ > FFlowField::InvalidHeight) { Transform.Position.Z = GroundZ; }

		// ③ PushToPrev
		PushToPrev(View.get<CTransformPrev>(Entity), View.get<CTransform>(Entity));
	});
}
