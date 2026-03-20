#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/Interface/Data/Targetable.h"
#include "Utils/Interface/Data/Damageable.h"
#include "TPSDestroyableBase.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnBaseDestroyed);

/**
 * 파괴 가능 기지 베이스
 * - HP 시스템: TakeDamage → HP 감소 → 0이면 파괴
 * - 파괴 시 OnBaseDestroyedDelegate 브로드캐스트
 * - 자식: ATPSAllyBase (아군), ATPSEnemyBase (적)
 */
UCLASS(Abstract)
class TPS_API ATPSDestroyableBase : public AActor, public ITargetable, public IDamageable
{
	GENERATED_BODY()

public:
	ATPSDestroyableBase();

	// ──────────── Getter ────────────

	FORCEINLINE float GetHealth() const { return CurrentHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }
	FORCEINLINE bool IsDestroyed() const { return bIsDestroyed; }

	// ──────────── 델리게이트 ────────────

	/** 기지 파괴 시 브로드캐스트 */
	FOnBaseDestroyed OnBaseDestroyedDelegate;

protected:
	virtual void BeginPlay() override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	/** 파괴 시 호출 — 자식에서 override하여 추가 처리 */
	virtual void OnBaseDestroyed();

#pragma region Component
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	TObjectPtr<class USceneComponent> SceneRootInst;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	TObjectPtr<class UStaticMeshComponent> MeshComponentInst;
#pragma endregion

#pragma region Health
	UPROPERTY(EditDefaultsOnly, Category = "Base")
	float MaxHealth = 5000.f;

	float CurrentHealth = 0.f;

	uint8 bIsDestroyed : 1 = false;
#pragma endregion

#pragma region ITargetable
	virtual FVector GetTargetLocation() const override { return GetActorLocation(); }
	virtual bool IsTargetable() const override { return CurrentHealth > 0.f; }
#pragma endregion

#pragma region IDamageable
	virtual float ReceiveDamage(float Damage, AActor* DamageCauser) override;
	virtual FVector GetDamageableLocation() const override { return GetActorLocation(); }
	virtual bool IsDamageable() const override { return !bIsDestroyed; }
#pragma endregion
};
