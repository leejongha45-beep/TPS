#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TPSWaveManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWaveChanged, int32, WaveIndex, int32, CycleIndex, int32, AdjustedSpawnCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCycleCompleted, int32, CompletedCycle);

/**
 * 무한 웨이브 라이프사이클 관리
 * - Config의 Waves 배열을 무한 순환 (Cycle)
 * - 순환마다 SpawnCountMultiplier 누적 적용
 * - 타이머 기반 — 전멸과 무관하게 CountdownTime마다 다음 웨이브 시작
 * - 동시 존재 상한 — MaxAliveEnemies 초과 시 대기열에 보류
 * - 지연 스폰 — 적 사망 시 여유만큼 대기열에서 추가 스폰
 * - Mass Entity 일괄 생성
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

	// ──────────── Getter ────────────

	/** 현재 웨이브 인덱스 — Config 배열 내 인덱스 (순환 시 리셋) */
	FORCEINLINE int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	/** 현재 순환 횟수 (0-based, 0 = 첫 회차) */
	FORCEINLINE int32 GetCurrentCycle() const { return CurrentCycle; }

	/** 누적 웨이브 번호 (1-based, 순환 무관하게 계속 증가) */
	FORCEINLINE int32 GetTotalWaveNumber() const { return TotalWaveNumber; }

	/** 현재 살아있는 적 수 */
	FORCEINLINE int32 GetAliveEnemies() const { return AliveEnemies; }

	/** 스폰 대기 중인 적 수 */
	FORCEINLINE int32 GetPendingSpawnCount() const { return PendingSpawnCount; }

	/** 총 킬 카운트 */
	FORCEINLINE int32 GetTotalKillCount() const { return TotalKillCount; }

	// ──────────── 델리게이트 ────────────

	/** 웨이브 시작 시 브로드캐스트 (웨이브 인덱스, 순환 회차, 보정된 스폰 수) */
	FOnWaveChanged OnWaveChangedDelegate;

	/** 순환 완료 시 브로드캐스트 (완료된 순환 번호) */
	FOnCycleCompleted OnCycleCompletedDelegate;

private:
	/** 카운트다운 완료 → 적 스폰 + 다음 웨이브 예약 */
	void OnCountdownFinished();

	/** 적 일괄 스폰 (동시 존재 상한 고려, 초과분은 PendingSpawnCount로 보류) */
	void SpawnWaveEnemies();

	/** 다음 웨이브 인덱스 진행 (순환 + Cycle 증가) */
	void AdvanceWaveIndex();

	/** 다음 웨이브 카운트다운 시작 */
	void StartNextWaveCountdown();

	/** 대기열에서 가능한 만큼 즉시 스폰 */
	void DrainPendingSpawns();

	/** Entity N개 생성 (공통 스폰 로직) — 실제 생성된 수 반환 */
	int32 SpawnEntities(int32 Count);

	/** 현재 Cycle 기준 보정된 스폰 수 계산 */
	int32 CalcAdjustedSpawnCount(int32 BaseCount) const;

	/** 동시 존재 상한 대비 추가 스폰 가능 수 */
	int32 GetSpawnCapacity() const;

	/** 스폰 포인트 비활성화 통보 핸들러 — 대기열 비율 감소 + 캐시 갱신 */
	void HandleSpawnPointsDeactivated(int32 DeactivatedCount, int32 ActiveCountBefore);

	UPROPERTY()
	TObjectPtr<class UTPSWaveConfig> WaveConfig;

	TWeakObjectPtr<UWorld> CachedWorld;

	/** Config 배열 내 인덱스 (0 ~ Waves.Num()-1, 순환 시 리셋) */
	int32 CurrentWaveIndex = 0;

	/** 순환 횟수 (0-based) */
	int32 CurrentCycle = 0;

	/** 누적 웨이브 번호 (1-based, 순환 무관) */
	int32 TotalWaveNumber = 0;

	/** 현재 월드에 살아있는 적 수 */
	int32 AliveEnemies = 0;

	/** 동시 존재 상한으로 보류된 스폰 대기 수 */
	int32 PendingSpawnCount = 0;

	int32 TotalKillCount = 0;

	/** 현재 웨이브의 스폰 포인트 캐시 (지연 스폰에서 재사용) */
	UPROPERTY()
	TArray<class ATPSEnemySpawnPoint*> CachedSpawnPoints;

	FTimerHandle CountdownTimerHandle;
};
