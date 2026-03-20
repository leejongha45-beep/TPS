#include "ECS/System/AISystem.h"
#include "ECS/Component/Components.h"
#include "Async/ParallelFor.h"

namespace
{

/** ② Write: 캐싱된 읽기값을 파라미터로 받아 Current에만 쓰기 */
void Write(CEnemyState& OutState, CMovement& OutMovement, CAIMode& OutAIMode,
           CWaypoint& OutWaypoint,
           EEnemyState NewState, const FVector& NewVelocity, EAIMode NewMode,
           int32 NewWaypointIndex)
{
	OutState.State = NewState;
	OutMovement.Velocity = NewVelocity;
	OutAIMode.Mode = NewMode;
	OutWaypoint.CurrentIndex = NewWaypointIndex;
}

/** ③ PushToPrev: 갱신된 Current → Prev 복사 (다음 프레임용) */
void PushToPrev(CEnemyStatePrev& OutStatePrev, CMovementPrev& OutMovementPrev,
                CAIModePrev& OutAIModePrev, CWaypointPrev& OutWaypointPrev,
                const CEnemyState& InState, const CMovement& InMovement,
                const CAIMode& InAIMode, const CWaypoint& InWaypoint)
{
	OutStatePrev.State = InState.State;
	OutMovementPrev.Velocity = InMovement.Velocity;
	OutMovementPrev.MaxSpeed = InMovement.MaxSpeed;
	OutAIModePrev.Mode = InAIMode.Mode;
	OutWaypointPrev.CurrentIndex = InWaypoint.CurrentIndex;
}

} // anonymous namespace

/**
 * Read는 Cached 지역변수로 값을 복사해야 하므로 함수 래핑 불가 — ParallelFor 본문에 직접 노출
 * Write/PushToPrev만 함수로 분리하여 가독성 확보
 */
void AISystem::Tick(entt::registry& Registry, const FVector& PlayerPosition,
                    float AttackRange, const TArray<FVector>& Waypoints,
                    float WaypointAcceptRadius, const FVector& BaseLocation)
{
	const float AttackRangeSq = AttackRange * AttackRange;
	const float AcceptRadiusSq = WaypointAcceptRadius * WaypointAcceptRadius;
	const int32 WaypointCount = Waypoints.Num();

	auto View = Registry.view<CEnemyState, CEnemyStatePrev, CMovement, CMovementPrev,
	                          CTransformPrev, CHealthPrev, CLODPrev,
	                          CAIMode, CAIModePrev, CNavTargetPrev,
	                          CWaypoint, CWaypointPrev>();

	// ── Entity 수집 — 단일 스레드에서 view 순회 (EnTT pool 구조 읽기) ──
	TArray<entt::entity, TInlineAllocator<3000>> Entities;
	Entities.Reserve(View.size_hint());
	for (auto Entity : View) { Entities.Add(Entity); }

	const int32 Count = Entities.Num();

	// ── ParallelFor: Entity별 독립 처리 ── [WorkerThread]
	ParallelFor(Count, [&View, &Entities, &PlayerPosition, &Waypoints, &BaseLocation,
	                     AttackRangeSq, AcceptRadiusSq, WaypointCount](int32 Index)
	{
		const entt::entity Entity = Entities[Index];

		// ① Read — Prev → 지역변수 (전부 상단에서 캐싱)
		const EEnemyState CachedState = View.get<CEnemyStatePrev>(Entity).State;
		if (CachedState == EEnemyState::Dying || CachedState == EEnemyState::Dead) { return; }

		const bool bCachedShouldTick = View.get<CLODPrev>(Entity).bShouldTick;
		if (!bCachedShouldTick) { return; }

		const FVector CachedPosition = View.get<CTransformPrev>(Entity).Position;
		const float CachedHealth     = View.get<CHealthPrev>(Entity).Current;
		const float CachedMaxHealth  = View.get<CHealthPrev>(Entity).Max;
		const float CachedMaxSpeed   = View.get<CMovementPrev>(Entity).MaxSpeed;
		const EAIMode CachedAIMode   = View.get<CAIModePrev>(Entity).Mode;
		const FVector CachedNavWP    = View.get<CNavTargetPrev>(Entity).NextWaypoint;
		int32 CachedWPIndex          = View.get<CWaypointPrev>(Entity).CurrentIndex;

		// ② 계산 — 지역변수만 사용
		EEnemyState NewState;
		FVector NewVelocity;
		EAIMode NewMode = CachedAIMode;
		int32 NewWPIndex = CachedWPIndex;

		if (CachedHealth <= 0.f)
		{
			NewState = EEnemyState::Dying;
			NewVelocity = FVector::ZeroVector;
		}
		else
		{
			const FVector ToPlayer = PlayerPosition - CachedPosition;
			const float DistSqToPlayer = ToPlayer.SizeSquared();

			// Rush → Chase 전환 판정
			if (CachedAIMode == EAIMode::Rush)
			{
				if (CachedHealth < CachedMaxHealth)
				{
					NewMode = EAIMode::Chase;
				}
				else if (DistSqToPlayer <= ECSConstants::AggroRadiusSq)
				{
					NewMode = EAIMode::Chase;
				}
			}

			// 공격 범위 판정 (Chase 모드에서만 플레이어 공격)
			if (NewMode == EAIMode::Chase && DistSqToPlayer <= AttackRangeSq)
			{
				NewState = (CachedState == EEnemyState::AttackCooldown ||
				            CachedState == EEnemyState::AttackReady ||
				            CachedState == EEnemyState::Attacking)
					? CachedState
					: EEnemyState::AttackCooldown;
				NewVelocity = FVector::ZeroVector;
			}
			else
			{
				NewState = EEnemyState::Moving;

				if (NewMode == EAIMode::Rush)
				{
					// 웨이포인트 도달 판정 → CurrentIndex 진행
					if (WaypointCount > 0 && CachedWPIndex < WaypointCount)
					{
						const FVector& Target = Waypoints[CachedWPIndex];
						const float DistSq = FVector::DistSquared2D(CachedPosition, Target);

						if (DistSq <= AcceptRadiusSq)
						{
							NewWPIndex = CachedWPIndex + 1;
						}
					}

					// NavMesh 경로 방향으로 이동 (UpdateNavTargets에서 쿼리된 결과)
					if (!CachedNavWP.IsNearlyZero())
					{
						const FVector Dir = (CachedNavWP - CachedPosition).GetSafeNormal();
						NewVelocity = Dir * CachedMaxSpeed;
					}
					else
					{
						// NavMesh 경로 없으면 기지 방향 직접 이동 (폴백)
						const FVector Dir = (BaseLocation - CachedPosition).GetSafeNormal2D();
						NewVelocity = Dir * CachedMaxSpeed;
					}
				}
				else // Chase
				{
					if (!CachedNavWP.IsNearlyZero())
					{
						const FVector Dir = (CachedNavWP - CachedPosition).GetSafeNormal();
						NewVelocity = Dir * CachedMaxSpeed;
					}
					else
					{
						NewVelocity = ToPlayer.GetSafeNormal() * CachedMaxSpeed;
					}
				}
			}
		}

		// ② Write
		Write(View.get<CEnemyState>(Entity), View.get<CMovement>(Entity),
		      View.get<CAIMode>(Entity), View.get<CWaypoint>(Entity),
		      NewState, NewVelocity, NewMode, NewWPIndex);

		// ③ PushToPrev
		PushToPrev(View.get<CEnemyStatePrev>(Entity), View.get<CMovementPrev>(Entity),
		           View.get<CAIModePrev>(Entity), View.get<CWaypointPrev>(Entity),
		           View.get<CEnemyState>(Entity), View.get<CMovement>(Entity),
		           View.get<CAIMode>(Entity), View.get<CWaypoint>(Entity));
	});
}
