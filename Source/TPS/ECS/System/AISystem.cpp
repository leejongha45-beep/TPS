#include "ECS/System/AISystem.h"
#include "ECS/Component/Components.h"
#include "Async/ParallelFor.h"

/** ② Write: 캐싱된 읽기값을 파라미터로 받아 Current에만 쓰기 */
void Write(CEnemyState& OutState, CMovement& OutMovement, EEnemyState NewState, const FVector& NewVelocity)
{
	OutState.State = NewState;
	OutMovement.Velocity = NewVelocity;
}

/** ③ PushToPrev: 갱신된 Current → Prev 복사 (다음 프레임용) */
void PushToPrev(CEnemyStatePrev& OutStatePrev, CMovementPrev& OutMovementPrev,
                const CEnemyState& InState, const CMovement& InMovement)
{
	OutStatePrev.State = InState.State;
	OutMovementPrev.Velocity = InMovement.Velocity;
	OutMovementPrev.MaxSpeed = InMovement.MaxSpeed;
}

/**
 * Read는 Cached 지역변수로 값을 복사해야 하므로 함수 래핑 불가 — ParallelFor 본문에 직접 노출
 * Write/PushToPrev만 함수로 분리하여 가독성 확보
 */
void AISystem::Tick(entt::registry& Registry, float DeltaTime, const FVector& PlayerPosition, float AttackRange)
{
	const float AttackRangeSq = AttackRange * AttackRange;

	auto View = Registry.view<CEnemyState, CEnemyStatePrev, CMovement, CMovementPrev, CTransformPrev, CHealthPrev, CLODPrev>();

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

		// 상태 결정 — 지역변수만 사용
		EEnemyState NewState;
		FVector NewVelocity;

		if (CachedHealth <= 0.f)
		{
			NewState = EEnemyState::Dying;
			NewVelocity = FVector::ZeroVector;
		}
		else
		{
			const FVector ToPlayer = PlayerPosition - CachedPosition;
			const float DistSq = ToPlayer.SizeSquared();

			if (DistSq <= AttackRangeSq)
			{
				NewState = (CachedState == EEnemyState::AttackCooldown || CachedState == EEnemyState::AttackReady || CachedState == EEnemyState::Attacking)
					? CachedState
					: EEnemyState::AttackCooldown;
				NewVelocity = FVector::ZeroVector;
			}
			else
			{
				NewState = EEnemyState::Moving;
				NewVelocity = ToPlayer.GetSafeNormal() * CachedMaxSpeed;
			}
		}

		// ② Write
		Write(View.get<CEnemyState>(Entity), View.get<CMovement>(Entity), NewState, NewVelocity);

		// ③ PushToPrev
		PushToPrev(View.get<CEnemyStatePrev>(Entity), View.get<CMovementPrev>(Entity),
		           View.get<CEnemyState>(Entity), View.get<CMovement>(Entity));
	});
}
