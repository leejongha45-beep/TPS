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
	ParallelFor(Count, [&](int32 Index)
	{
		const entt::entity Entity = Entities[Index];

		// ① Read: Prev → Cached 지역변수
		const EEnemyState CachedState = View.get<CEnemyStatePrev>(Entity).State;

		// Dying/Dead Entity는 AI 갱신 불필요
		if (CachedState == EEnemyState::Dying || CachedState == EEnemyState::Dead) { return; }

		// LOD 스킵 — 스킵 시 Write/PushToPrev 건너뜀 → Velocity/State 유지 (관성 이동)
		if (!View.get<CLODPrev>(Entity).bShouldTick) { return; }

		const FVector CachedPosition = View.get<CTransformPrev>(Entity).Position;
		const float CachedHealth = View.get<CHealthPrev>(Entity).Current;
		const float CachedMaxSpeed = View.get<CMovementPrev>(Entity).MaxSpeed;
		const EAIMode CachedAIMode = View.get<CAIModePrev>(Entity).Mode;

		// 상태 결정 — 지역변수만 사용
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

			// ── Rush → Chase 전환 판정 ──
			if (CachedAIMode == EAIMode::Rush)
			{
				// 피격 감지: 현재 HP < MaxHP (이미 데미지를 받은 적)
				const float CachedMaxHealth = View.get<CHealthPrev>(Entity).Max;
				if (CachedHealth < CachedMaxHealth)
				{
					NewMode = EAIMode::Chase;
				}
				// 플레이어 탐지 범위 진입
				else if (DistSqToPlayer <= ECSConstants::AggroRadiusSq)
				{
					NewMode = EAIMode::Chase;
				}
			}

			// ── 공격 범위 판정 (Chase 모드에서만 플레이어 공격) ──
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
					// Flow Field 방향 조회
					const FVector2f FlowDir = BaseFlowField.LookupDirection(
						CachedPosition.X, CachedPosition.Y);

					if (!FlowDir.IsNearlyZero())
					{
						NewVelocity = FVector(FlowDir.X, FlowDir.Y, 0.f) * CachedMaxSpeed;
					}
					else
					{
						// 그리드 밖 폴백 — 기지 방향 직선
						const FVector ToBase = BaseLocation - CachedPosition;
						NewVelocity = ToBase.GetSafeNormal() * CachedMaxSpeed;
					}
				}
				else // Chase
				{
					// NavMesh 웨이포인트 방향
					const FVector& Waypoint = View.get<CNavTargetPrev>(Entity).NextWaypoint;
					if (!Waypoint.IsNearlyZero())
					{
						const FVector ToWaypoint = Waypoint - CachedPosition;
						NewVelocity = ToWaypoint.GetSafeNormal() * CachedMaxSpeed;
					}
					else
					{
						// NavTarget 없으면 직선 폴백
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
