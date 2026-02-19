#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSEnemyHealthComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnEnemyDeath);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEnemyDamaged, float /* InDamage */, float /* InRemainingHealth */);

/**
 * 적 체력 관리 컴포넌트
 * - APawn::TakeDamage에서 호출
 * - 데미지 적용 → Mass Fragment 동기화는 소유 Pawn이 담당
 * - 사망 판정 → 델리게이트 브로드캐스트
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSEnemyHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Mass Fragment에서 체력 초기값 로드 */
	void InitFromFragment(float InMaxHealth, float InCurrentHealth);

	/** 데미지 적용 → 체력 감산 → 사망 판정 */
	void ApplyDamage(float InDamage);

	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }
	FORCEINLINE bool IsDead() const { return bIsDead; }

	FOnEnemyDeath OnEnemyDeathDelegate;
	FOnEnemyDamaged OnEnemyDamagedDelegate;

protected:
	float MaxHealth = 50.f;
	float CurrentHealth = 50.f;
	uint8 bIsDead : 1 = false;
};
