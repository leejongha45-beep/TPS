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

int32 DamageSystem::Tick(entt::registry& Registry,
                         TArray<FDamageEvent>& DamageQueue,
                         TArray<entt::entity> (&InstanceToEntityPerLOD)[HISM_LOD_COUNT],
                         TArray<FHitEffectRequest>& OutHitEffects)
{
	int32 PlayerKillCount = 0;

	for (const FDamageEvent& Event : DamageQueue)
	{
		// ① Read: LOD + InstanceIndex 유효성 검증
		if (Event.LODLevel >= HISM_LOD_COUNT) { continue; }

		const TArray<entt::entity>& InstanceToEntity = InstanceToEntityPerLOD[Event.LODLevel];
		if (Event.InstanceIndex < 0 || Event.InstanceIndex >= InstanceToEntity.Num()) { continue; }

		const entt::entity Entity = InstanceToEntity[Event.InstanceIndex];
		if (!Registry.valid(Entity)) { continue; }

		// Dying/Dead 스킵
		const EEnemyState CachedState = Registry.get<CEnemyStatePrev>(Entity).State;
		if (CachedState == EEnemyState::Dying || CachedState == EEnemyState::Dead) { continue; }

		// 이미 체력 0 스킵
		const float CachedHealth = Registry.get<CHealthPrev>(Entity).Current;
		if (CachedHealth <= 0.f) { continue; }

		// ② Write
		CHealth& Health = Registry.get<CHealth>(Entity);
		Write(Health, Event.Damage);

		// ③ PushToPrev
		PushToPrev(Registry.get<CHealthPrev>(Entity), Health);

		// ④ 히트 이펙트 요청
		if (!Event.HitLocation.IsZero())
		{
			OutHitEffects.Add({ Event.HitLocation, Event.HitNormal });
		}

		// ⑤ 플레이어 킬 감지
		if (Event.bFromPlayer && Health.Current <= 0.f)
		{
			++PlayerKillCount;
		}
	}

	DamageQueue.Reset();
	return PlayerKillCount;
}
