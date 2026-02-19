#include "Enemy/Component/TPSEnemyHealthComponent.h"

void UTPSEnemyHealthComponent::InitFromFragment(float InMaxHealth, float InCurrentHealth)
{
	MaxHealth = InMaxHealth;
	CurrentHealth = InCurrentHealth;
	bIsDead = (CurrentHealth <= 0.f);
}

void UTPSEnemyHealthComponent::ApplyDamage(float InDamage)
{
	// ① 이미 죽었으면 무시
	if (bIsDead) return;

	// ② 체력 감산 (0 미만 방지)
	CurrentHealth = FMath::Max(0.f, CurrentHealth - InDamage);

	// ③ 피격 델리게이트 (히트 이펙트 등에서 수신)
	OnEnemyDamagedDelegate.Broadcast(InDamage, CurrentHealth);

	// ④ 사망 판정
	if (CurrentHealth <= 0.f)
	{
		bIsDead = true;
		OnEnemyDeathDelegate.Broadcast();
	}
}
