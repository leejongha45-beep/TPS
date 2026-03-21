#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSPsychoSyncComponent.generated.h"

/** 사이코싱크 단계 */
UENUM(BlueprintType)
enum class EPsychoPhase : uint8
{
	Phase0				UMETA(DisplayName = "Base"),
	Phase1_InfiniteAmmo	UMETA(DisplayName = "Infinite Ammo"),
	Phase2_Penetration	UMETA(DisplayName = "Penetration"),
};

/** 페이즈 변경 시 브로드캐스트 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPsychoPhaseChanged, EPsychoPhase /* NewPhase */, EPsychoPhase /* OldPhase */);

/**
 * 사이코싱크 컴포넌트 — 에반게리온 모티브
 * - 연속 킬(스트릭)로 무기 동기화율 상승
 * - Phase 0: 무능력 → Phase 1(20킬): 무한탄창 → Phase 2(50킬): 관통탄
 * - 10초 킬 없으면 1단계 하락, 사망 시 완전 초기화
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSPsychoSyncComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** EnemyManagerSubsystem 델리게이트 바인딩 */
	void Initialize();

	/** 완전 초기화 (사망 시 호출) */
	void Reset();

	/** 킬 이벤트 수신 */
	void OnEnemyKilled(int32 KillCount);

	FORCEINLINE EPsychoPhase GetCurrentPhase() const { return CurrentPhase; }
	FORCEINLINE bool HasInfiniteAmmo() const { return CurrentPhase >= EPsychoPhase::Phase1_InfiniteAmmo; }
	FORCEINLINE bool HasPenetration() const { return CurrentPhase >= EPsychoPhase::Phase2_Penetration; }
	FORCEINLINE int32 GetKillStreak() const { return KillStreak; }

	FOnPsychoPhaseChanged OnPsychoPhaseChangedDelegate;

protected:
	void OnDecayTimerExpired();
	void ResetDecayTimer();
	void SetPhase(EPsychoPhase NewPhase);

	/** FireComponent에 상태 동기화 */
	void SyncFireComponent();

	EPsychoPhase CurrentPhase = EPsychoPhase::Phase0;
	int32 KillStreak = 0;
	FTimerHandle DecayTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "PsychoSync")
	int32 Phase1Threshold = 20;

	UPROPERTY(EditDefaultsOnly, Category = "PsychoSync")
	int32 Phase2Threshold = 50;

	UPROPERTY(EditDefaultsOnly, Category = "PsychoSync")
	float DecayInterval = 10.f;
};
