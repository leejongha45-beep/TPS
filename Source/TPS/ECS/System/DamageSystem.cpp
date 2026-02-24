#include "ECS/System/DamageSystem.h"
#include "ECS/Component/Components.h"

namespace
{

/** ② Write: 데미지 적용 (0 클램프) */
void Write(CHealth& OutHealth, float Damage)
{
	OutHealth.Current = FMath::Max(OutHealth.Current - Damage, 0.f);
}

/** ③ PushToPrev: 갱신된 체력 → Prev 반영 */
void PushToPrev(CHealthPrev& OutPrev, const CHealth& InCurrent)
{
	OutPrev.Current = InCurrent.Current;
}

} // anonymous namespace

void DamageSystem::Tick(entt::registry& Registry,
                        TArray<FDamageEvent>& DamageQueue,
                        const TArray<entt::entity>& InstanceToEntity)
{
	for (const FDamageEvent& Event : DamageQueue)
	{
		// ① Read: 큐 이벤트 → InstanceIndex 유효성 검증
		if (Event.InstanceIndex < 0 || Event.InstanceIndex >= InstanceToEntity.Num()) { continue; }

		const entt::entity Entity = InstanceToEntity[Event.InstanceIndex];
		if (!Registry.valid(Entity)) { continue; }

		// Dying/Dead 스킵
		const EEnemyState CachedState = Registry.get<CEnemyStatePrev>(Entity).State;
		if (CachedState == EEnemyState::Dying || CachedState == EEnemyState::Dead) { continue; }

		// 이미 체력 0 스킵 — 같은 프레임 다중 히트 시 첫 히트에서 0 도달하면 이후 불필요
		const float CachedHealth = Registry.get<CHealthPrev>(Entity).Current;
		if (CachedHealth <= 0.f) { continue; }

		// ② Write
		CHealth& Health = Registry.get<CHealth>(Entity);
		Write(Health, Event.Damage);

		// ③ PushToPrev
		PushToPrev(Registry.get<CHealthPrev>(Entity), Health);
	}

	DamageQueue.Reset();
}
