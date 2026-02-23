#include "ECS/System/MovementSystem.h"
#include "ECS/Component/Components.h"
#include "Async/ParallelFor.h"

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

/**
 * Read는 Cached 지역변수로 값을 복사해야 하므로 함수 래핑 불가 — ParallelFor 본문에 직접 노출
 * Write/PushToPrev만 함수로 분리하여 가독성 확보
 */
void MovementSystem::Tick(entt::registry& Registry, float DeltaTime)
{
	auto View = Registry.view<CTransform, CTransformPrev, CMovementPrev, CEnemyStatePrev>();

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
		const EEnemyState CachedState = View.get<CEnemyStatePrev>(Entity).State;
		if (CachedState != EEnemyState::Moving) { return; }

		const FVector CachedVelocity = View.get<CMovementPrev>(Entity).Velocity;

		// ② Write
		Write(View.get<CTransform>(Entity), DeltaTime, CachedVelocity);

		// ③ PushToPrev
		PushToPrev(View.get<CTransformPrev>(Entity), View.get<CTransform>(Entity));
	});
}
