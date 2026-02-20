#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/Interface/Data/Targetable.h"
#include "TPSAllyBase.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnBaseDestroyed);

/**
 * 아군 기지 액터
 * - 적 AI의 기본 진격 목표 (ITargetable)
 * - HP 시스템: TakeDamage → HP 감소 → 0이면 파괴
 * - 파괴 시 OnBaseDestroyedDelegate 브로드캐스트
 *
 * 향후 확장: 파괴 연출, 스폰 포인트 비활성화 연동, HUD 체력바
 */
UCLASS()
class TPS_API ATPSAllyBase : public AActor, public ITargetable
{
	GENERATED_BODY()

public:
	ATPSAllyBase();

	// ──────────── Getter ────────────

	FORCEINLINE float GetHealth() const { return CurrentHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

	// ──────────── 델리게이트 ────────────

	/** 기지 파괴 시 브로드캐스트 */
	FOnBaseDestroyed OnBaseDestroyedDelegate;

protected:
	virtual void BeginPlay() override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

#pragma region Component
	/** 씬 루트 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	TObjectPtr<class USceneComponent> SceneRootInst;

	/** 기지 메시 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	TObjectPtr<class UStaticMeshComponent> MeshComponentInst;
#pragma endregion

#pragma region Health
	/** 기지 최대 체력 */
	UPROPERTY(EditDefaultsOnly, Category = "Base")
	float MaxHealth = 5000.f;

	/** 현재 체력 (BeginPlay에서 MaxHealth로 초기화) */
	float CurrentHealth = 0.f;

	/** HP 0 도달 시 호출 */
	void OnBaseDestroyed();

	/** 파괴 여부 (중복 호출 방지) */
	uint8 bIsDestroyed : 1 = false;
#pragma endregion

#pragma region ITargetable
	virtual FVector GetTargetLocation() const override { return GetActorLocation(); }
	virtual bool IsTargetable() const override { return CurrentHealth > 0.f; }
#pragma endregion
};
