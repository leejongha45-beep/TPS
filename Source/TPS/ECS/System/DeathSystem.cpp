#include "ECS/System/DeathSystem.h"
#include "ECS/Component/Components.h"
#include "Async/ParallelFor.h"

/** ② Write: Dying → Dead 전환 */
void Write(CEnemyState& OutState)
{
	OutState.State = EEnemyState::Dead;
}

/** ③ PushToPrev: CEnemyState → CEnemyStatePrev */
void PushToPrev(CEnemyStatePrev& OutPrev, const CEnemyState& InCurrent)
{
	OutPrev.State = InCurrent.State;
}

void DeathSystem::Tick(entt::registry& Registry)
{
	auto View = Registry.view<CEnemyState, CEnemyStatePrev, CAnimationPrev>();

	// ── Entity 수집 ──
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
		if (CachedState != EEnemyState::Dying) { return; }

		const float CachedAnimTime = View.get<CAnimationPrev>(Entity).AnimTime;
		if (CachedAnimTime < ECSConstants::DeathAnimDuration) { return; }

		// ② Write
		Write(View.get<CEnemyState>(Entity));

		// ③ PushToPrev
		PushToPrev(View.get<CEnemyStatePrev>(Entity), View.get<CEnemyState>(Entity));
	});
}
