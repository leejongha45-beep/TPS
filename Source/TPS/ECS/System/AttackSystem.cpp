#include "ECS/System/AttackSystem.h"
#include "ECS/Component/Components.h"
#include "Utils/Interface/Data/Damageable.h"

namespace
{

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

} // anonymous namespace

void AttackSystem::Tick(entt::registry& Registry, float DeltaTime, IDamageable* PlayerDamageable)
{
	float TotalDamage = 0.f;

	auto View = Registry.view<CAttack, CAttackPrev, CEnemyState, CEnemyStatePrev, CAnimationPrev>();

	for (auto Entity : View)
	{
		// ① Read: Prev → Cached 지역변수
		const EEnemyState CachedState   = View.get<CEnemyStatePrev>(Entity).State;
		const float CachedTimer         = View.get<CAttackPrev>(Entity).CooldownTimer;
		const float CachedDamage        = View.get<CAttackPrev>(Entity).Damage;
		const float CachedCooldown      = View.get<CAttackPrev>(Entity).Cooldown;
		const float CachedAnimTime      = View.get<CAnimationPrev>(Entity).AnimTime;

		// AttackCooldown: 쿨다운 틱 → 만료 시 AttackReady 전환
		if (CachedState == EEnemyState::AttackCooldown)
		{
			float NewTimer = CachedTimer - DeltaTime;
			if (NewTimer <= 0.f)
			{
				Write(View.get<CAttack>(Entity), View.get<CEnemyState>(Entity),
				      0.f, EEnemyState::AttackReady);
			}
			else
			{
				Write(View.get<CAttack>(Entity), View.get<CEnemyState>(Entity),
				      NewTimer, EEnemyState::AttackCooldown);
			}
		}
		// AttackReady: 준비 모션 종료 → Attacking 전환
		else if (CachedState == EEnemyState::AttackReady)
		{
			if (CachedAnimTime >= ECSConstants::AttackReadyDuration)
			{
				Write(View.get<CAttack>(Entity), View.get<CEnemyState>(Entity),
				      CachedTimer, EEnemyState::Attacking);
			}
			else
			{
				Write(View.get<CAttack>(Entity), View.get<CEnemyState>(Entity),
				      CachedTimer, EEnemyState::AttackReady);
			}
		}
		// Attacking: 공격 모션 종료 → 데미지 집계 + AttackCooldown 전환
		else if (CachedState == EEnemyState::Attacking)
		{
			if (CachedAnimTime >= ECSConstants::AttackDuration)
			{
				TotalDamage += CachedDamage;
				Write(View.get<CAttack>(Entity), View.get<CEnemyState>(Entity),
				      CachedCooldown, EEnemyState::AttackCooldown);
			}
			else
			{
				Write(View.get<CAttack>(Entity), View.get<CEnemyState>(Entity),
				      CachedTimer, EEnemyState::Attacking);
			}
		}
		else { continue; }

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
