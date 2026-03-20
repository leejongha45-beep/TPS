// TPSWaveSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "Wave/WaveTypes.h"
#include "ECS/Data/EnemySpawnParams.h"
#include "TPSWaveSubsystem.generated.h"

/**
 * 웨이브 오케스트레이터
 *
 * - 트리클: TrickleSpawnInterval마다 소그룹 직접 QueueSpawn
 * - 빅웨이브: BigWavePeriod마다 Alert → 일괄 QueueSpawn → Trickle 복귀
 * - EnemyManagerSubsystem::QueueSpawn()에 직접 전달 (자체 큐 없음)
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

	void StartWaveSystem();
	void StopWaveSystem();

	FORCEINLINE int32 GetCurrentWaveLevel() const { return CurrentWaveLevel; }
	FORCEINLINE EWavePhase GetCurrentPhase() const { return CurrentPhase; }

protected:
	// ── 상태 ──
	EWavePhase CurrentPhase = EWavePhase::Idle;
	int32 CurrentWaveLevel = 0;
	float ElapsedTime = 0.f;
	float LastTrickleTime = 0.f;
	float LastBigWaveTime = 0.f;
	uint8 bIsActive : 1 = false;

	// ── 캐시 ──
	TObjectPtr<const class UTPSWaveSettings> CachedSettings = nullptr;
	TObjectPtr<class UTPSEnemyTypeDataAsset> CachedEnemyType = nullptr;
	TArray<TWeakObjectPtr<class ATPSEnemySpawnPoint>> CachedSpawnPoints;

	// ── 내부 함수 ──

	/** 트리클 — 주기마다 소그룹 직접 QueueSpawn */
	void TickTrickleSpawn();

	/** 빅웨이브 일괄 스폰 — Count만큼 즉시 QueueSpawn */
	void SpawnBatch(int32 Count);

	/** DataAsset → FEnemySpawnParams */
	FEnemySpawnParams BuildSpawnParams(const class UTPSEnemyTypeDataAsset* Type) const;

	/** 월드에서 스폰 포인트 수집 */
	void CollectSpawnPoints();

	/** 랜덤 스폰 위치 반환 */
	FVector PickRandomSpawnLocation() const;
};
