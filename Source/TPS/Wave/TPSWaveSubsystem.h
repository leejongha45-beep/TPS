// TPSWaveSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "Wave/WaveTypes.h"
#include "ECS/Data/EnemySpawnParams.h"
#include "TPSWaveSubsystem.generated.h"

/** 스폰 예약 엔트리 — 시간순 처리 */
struct FWaveSpawnRequest
{
	FEnemySpawnParams Params;
	float ScheduledTime = 0.f;
};

/**
 * 웨이브 시스템 오케스트레이터 — 트리클 스폰 + 빅웨이브 제어
 *
 * - 트리클: TrickleSpawnInterval마다 소그룹 스폰 (항상 동작)
 * - 빅웨이브: BigWavePeriod마다 대규모 그룹 분할 스폰
 * - 수식 기반 스케일링: 웨이브 레벨 × 배율로 적 스탯 증가
 * - PendingSpawnQueue에 시간 예약 → FlushReadySpawns에서 EnemyManagerSubsystem에 전달
 */
UCLASS()
class TPS_API UTPSWaveSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** 전투 시작 — 트리클 + 빅웨이브 타이머 가동 */
	void StartWaveSystem();

	/** 전투 종료 — 모든 타이머 중단, 큐 비우기 */
	void StopWaveSystem();

	FORCEINLINE int32 GetCurrentWaveLevel() const { return CurrentWaveLevel; }
	FORCEINLINE EWavePhase GetCurrentPhase() const { return CurrentPhase; }

protected:
	// ── 상태 ──
	EWavePhase CurrentPhase = EWavePhase::Idle;
	int32 CurrentWaveLevel = 0;
	float ElapsedTime = 0.f;
	float LastTrickleSpawnTime = 0.f;
	float LastBigWaveTime = 0.f;
	float BigWaveAlertEndTime = 0.f;
	uint8 bIsActive : 1 = false;

	// ── 스폰 큐 ──
	TArray<FWaveSpawnRequest> PendingSpawnQueue;
	int32 BigWaveGroupsRemaining = 0;
	float NextBigWaveGroupTime = 0.f;

	// ── 캐시 (Initialize에서 세팅) ──
	TArray<TObjectPtr<class UTPSEnemyTypeDataAsset>> LoadedEnemyTypes;
	FVector AllyBaseLocation = FVector::ZeroVector;

	// ── 내부 함수 ──

	/** 트리클 스폰 — 매 TrickleSpawnInterval마다 소그룹 스폰 예약 */
	void TickTrickleSpawn();

	/** 빅웨이브 그룹 스폰 — 남은 그룹이 있으면 시간 도래 시 스폰 예약 */
	void TickBigWave();

	/**
	 * 스폰 그룹 예약 — PendingSpawnQueue에 시간차 스폰 요청 적재
	 * @param Count          그룹 내 적 수
	 * @param AtTime         그룹 시작 시각 (ElapsedTime 기준)
	 * @param StaggerDelay   그룹 내 적 간 스폰 딜레이
	 */
	void ScheduleSpawnGroup(int32 Count, float AtTime, float StaggerDelay);

	/** 예약 시각이 도래한 스폰 요청을 EnemyManagerSubsystem에 전달 */
	void FlushReadySpawns();

	/**
	 * DataAsset 스탯 × 스케일링 배율 적용하여 FEnemySpawnParams 생성
	 * @param Type   적 타입 DataAsset
	 */
	FEnemySpawnParams BuildSpawnParams(const class UTPSEnemyTypeDataAsset* Type) const;

	/** AllyBase 중심 스폰 링 내 랜덤 위치 산출 */
	FVector CalculateSpawnPosition() const;

	/** 현재 웨이브 레벨에서 등장 가능한 타입 중 가중치 랜덤 선택 */
	const class UTPSEnemyTypeDataAsset* SelectEnemyType() const;

	/** 스케일링 배율 산출: 1.0 + CurrentWaveLevel * BaseMultiplierPerWave */
	FORCEINLINE float GetStatMultiplier(float BaseMultiplierPerWave) const
	{
		return 1.f + static_cast<float>(CurrentWaveLevel) * BaseMultiplierPerWave;
	}
};
