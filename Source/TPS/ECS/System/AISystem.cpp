#include "ECS/System/AISystem.h"
#include "ECS/Component/Components.h"
#include "ECS/System/FlowFieldSystem.h"
#include "Async/ParallelFor.h"

namespace
{

/** ② Write: 캐싱된 읽기값을 파라미터로 받아 Current에만 쓰기 */
void Write(CEnemyState& OutState, CMovement& OutMovement, CAIMode& OutAIMode,
           EEnemyState NewState, const FVector& NewVelocity, EAIMode NewMode)
{
	OutState.State = NewState;
	OutMovement.Velocity = NewVelocity;
	OutAIMode.Mode = NewMode;
}

/** ③ PushToPrev: 갱신된 Current → Prev 복사 (다음 프레임용) */
void PushToPrev(CEnemyStatePrev& OutStatePrev, CMovementPrev& OutMovementPrev,
                CAIModePrev& OutAIModePrev,
                const CEnemyState& InState, const CMovement& InMovement,
                const CAIMode& InAIMode)
{
	OutStatePrev.State = InState.State;
	OutMovementPrev.Velocity = InMovement.Velocity;
	OutMovementPrev.MaxSpeed = InMovement.MaxSpeed;
	OutAIModePrev.Mode = InAIMode.Mode;
}

} // anonymous namespace

/**
 * Read는 Cached 지역변수로 값을 복사해야 하므로 함수 래핑 불가 — ParallelFor 본문에 직접 노출
 * Write/PushToPrev만 함수로 분리하여 가독성 확보
 */
void AISystem::Tick(entt::registry& Registry, const FVector& PlayerPosition,
                    float AttackRange, const FFlowField& BaseFlowField,
                    const FVector& BaseLocation)
{
	const float AttackRangeSq = AttackRange * AttackRange;

	auto View = Registry.view<CEnemyState, CEnemyStatePrev, CMovement, CMovementPrev,
	                          CTransformPrev, CHealthPrev, CLODPrev,
	                          CAIMode, CAIModePrev, CNavTargetPrev>();

	// ── Entity 수집 — 단일 스레드에서 view 순회 (EnTT pool 구조 읽기) ──
	TArray<entt::entity, TInlineAllocator<3000>> Entities;
	Entities.Reserve(View.size_hint());
	for (auto Entity : View) { Entities.Add(Entity); }

	const int32 Count = Entities.Num();

	// ── ParallelFor: Entity별 독립 처리 ── [WorkerThread]
	ParallelFor(Count, [&View, &Entities, &PlayerPosition, &BaseFlowField, &BaseLocation, AttackRangeSq](int32 Index)
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
		const FVector CachedWaypoint = View.get<CNavTargetPrev>(Entity).NextWaypoint;

		// ② 계산 — 지역변수만 사용
		EEnemyState NewState;
		FVector NewVelocity;
		EAIMode NewMode = CachedAIMode;

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
					const FVector2f FlowDir = BaseFlowField.LookupDirection(
						CachedPosition.X, CachedPosition.Y);

					if (!FlowDir.IsNearlyZero())
					{
						NewVelocity = FVector(FlowDir.X, FlowDir.Y, 0.f) * CachedMaxSpeed;
					}
					else
					{
						const FVector ToBase = BaseLocation - CachedPosition;
						NewVelocity = ToBase.GetSafeNormal() * CachedMaxSpeed;
					}
				}
				else // Chase
				{
					if (!CachedWaypoint.IsNearlyZero())
					{
						const FVector ToWaypoint = CachedWaypoint - CachedPosition;
						NewVelocity = ToWaypoint.GetSafeNormal() * CachedMaxSpeed;
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
		      View.get<CAIMode>(Entity), NewState, NewVelocity, NewMode);

		// ③ PushToPrev
		PushToPrev(View.get<CEnemyStatePrev>(Entity), View.get<CMovementPrev>(Entity),
		           View.get<CAIModePrev>(Entity),
		           View.get<CEnemyState>(Entity), View.get<CMovement>(Entity),
		           View.get<CAIMode>(Entity));
	});
}
