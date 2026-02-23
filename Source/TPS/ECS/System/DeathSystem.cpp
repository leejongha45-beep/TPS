#include "ECS/System/DeathSystem.h"
#include "ECS/Component/Components.h"

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

	for (auto Entity : View)
	{
		// ① Read: Prev → Cached 지역변수
		const EEnemyState CachedState = View.get<CEnemyStatePrev>(Entity).State;
		if (CachedState != EEnemyState::Dying) { continue; }

		const float CachedAnimTime = View.get<CAnimationPrev>(Entity).AnimTime;
		if (CachedAnimTime < ECSConstants::DeathAnimDuration) { continue; }

		// ② Write
		Write(View.get<CEnemyState>(Entity));

		// ③ PushToPrev
		PushToPrev(View.get<CEnemyStatePrev>(Entity), View.get<CEnemyState>(Entity));
	}
}
