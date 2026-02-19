#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TPSWaveManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveChanged, int32, WaveIndex, int32, TotalWaves);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCleared);

/**
 * 웨이브 라이프사이클 관리
 * - 카운트다운 → 스포닝 → 전투 → 클리어 → 다음 웨이브
 * - Mass Entity 일괄 생성/파괴
 * - 킬 카운트 추적
 */
UCLASS()
class TPS_API UTPSWaveManager : public UObject
{
	GENERATED_BODY()

public:
	/** 초기화 — Config 설정 + World 캐싱 */
	void Initialize(UWorld* InWorld, class UTPSWaveConfig* InConfig);

	/** 웨이브 시스템 시작 (첫 웨이브 카운트다운) */
	void StartWaves();

	/** 적 사망 통보 — DeathProcessor에서 호출 */
	void NotifyEnemyKilled();

	/** 현재 웨이브 인덱스 (0-based) */
	FORCEINLINE int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	/** 남은 적 수 */
	FORCEINLINE int32 GetRemainingEnemies() const { return RemainingEnemies; }

	/** 총 킬 카운트 */
	FORCEINLINE int32 GetTotalKillCount() const { return TotalKillCount; }

	FOnWaveChanged OnWaveChangedDelegate;
	FOnAllWavesCleared OnAllWavesClearedDelegate;

private:
	/** 카운트다운 완료 → 적 스폰 */
	void OnCountdownFinished();

	/** 적 일괄 스폰 (Mass Entity 생성) */
	void SpawnWaveEnemies();

	/** 웨이브 클리어 체크 → 다음 웨이브 or 올 클리어 */
	void CheckWaveClear();

	UPROPERTY()
	TObjectPtr<class UTPSWaveConfig> WaveConfig;

	TWeakObjectPtr<UWorld> CachedWorld;

	int32 CurrentWaveIndex = 0;
	int32 RemainingEnemies = 0;
	int32 TotalKillCount = 0;

	FTimerHandle CountdownTimerHandle;
};
