#include "ECS/System/AttackSystem.h"
#include "ECS/Component/Components.h"
#include "Utils/Interface/Data/Damageable.h"

/** ② Write: 쿨다운 타이머 + 상태 갱신 */
void Write(CAttack& OutAttack, CEnemyState& OutState, float NewTimer, EEnemyState NewEnemyState)
{
	OutAttack.CooldownTimer = NewTimer;
	OutState.State = NewEnemyState;
}

/** ③ PushToPrev: 갱신된 Current → Prev 복사 */
void PushToPrev(CAttackPrev& OutAttackPrev, CEnemyStatePrev& OutStatePrev,
                const CAttack& InAttack, const CEnemyState& InState)
{
	OutAttackPrev.Damage        = InAttack.Damage;
	OutAttackPrev.Cooldown      = InAttack.Cooldown;
	OutAttackPrev.CooldownTimer = InAttack.CooldownTimer;
	OutStatePrev.State          = InState.State;
}

void AttackSystem::Tick(entt::registry& Registry, float DeltaTime, IDamageable* PlayerDamageable)
{
	float TotalDamage = 0.f;

	auto View = Registry.view<CAttack, CAttackPrev, CEnemyState, CEnemyStatePrev>();

	for (auto Entity : View)
	{
		// ① Read: Prev → Cached 지역변수
		const EEnemyState CachedState    = View.get<CEnemyStatePrev>(Entity).State;
		const float CachedTimer          = View.get<CAttackPrev>(Entity).CooldownTimer;
		const float CachedDamage         = View.get<CAttackPrev>(Entity).Damage;
		const float CachedCooldown       = View.get<CAttackPrev>(Entity).Cooldown;

		// 공격 상태가 아닌 Entity는 스킵
		if (CachedState != EEnemyState::AttackReady && CachedState != EEnemyState::Attacking) { continue; }

		// 쿨다운 틱
		float NewTimer = CachedTimer - DeltaTime;
		if (NewTimer <= 0.f)
		{
			TotalDamage += CachedDamage;
			NewTimer += CachedCooldown;    // 오버슈트 보상

			// ② Write — 쿨다운 만료 = 공격 실행
			Write(View.get<CAttack>(Entity), View.get<CEnemyState>(Entity),
			      NewTimer, EEnemyState::Attacking);
		}
		else
		{
			// ② Write — 쿨다운 틱 중 = 준비 상태
			Write(View.get<CAttack>(Entity), View.get<CEnemyState>(Entity),
			      NewTimer, EEnemyState::AttackReady);
		}

		// ③ PushToPrev
		PushToPrev(View.get<CAttackPrev>(Entity), View.get<CEnemyStatePrev>(Entity),
		           View.get<CAttack>(Entity), View.get<CEnemyState>(Entity));
	}

	// 프레임당 1회 집계 호출
	if (TotalDamage > 0.f && PlayerDamageable && PlayerDamageable->IsDamageable())
	{
		PlayerDamageable->ReceiveDamage(TotalDamage, nullptr);
	}
}
