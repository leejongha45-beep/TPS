#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Utils/UENUM/SwarmTeam.h"
#include "Wave/WaveTypes.h"
#include "TPSSwarmSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FSwarm
{
	GENERATED_BODY()

	int32 SwarmID = INDEX_NONE;
	FVector Position = FVector::ZeroVector;
	ESwarmTeam Team = ESwarmTeam::Enemy;

	int32 TroopCount = 0;
	float UnitMaxHP = 100.f;
	float CurrentUnitHP = 100.f;
	float AttackPower = 10.f;
	float MoveSpeed = 300.f;

	int32 WaypointIndex = 0;
	int32 RouteIndex = 0;
	bool bUnfolded = false;
	bool bEngaged = false;
};

/**
 * 군집 시스템 — 추상 전투 + 플레이어 근접 시 Unfold/Fold
 * - 군집끼리 만나면 HP 감산 (정수 연산)
 * - 플레이어 접근 시 ECS 엔티티 / NPC로 펼치기
 * - 플레이어 이탈 시 다시 군집으로 접기
 */
UCLASS()
class TPS_API UTPSSwarmSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// ──────────── UTickableWorldSubsystem ────────────
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// ──────────── 웨이브 시스템 (WaveSubsystem 역할 흡수) ────────────
	void StartWaveSystem();
	void StopWaveSystem();

	FORCEINLINE int32 GetCurrentWaveLevel() const { return CurrentWaveLevel; }
	FORCEINLINE EWavePhase GetCurrentPhase() const { return CurrentPhase; }

	// ──────────── 군집 관리 ────────────
	void AddSwarm(const FSwarm& InSwarm);
	void RemoveSwarm(int32 Index);

	int32 GetSwarmCount() const { return Swarms.Num(); }
	const TArray<FSwarm>& GetSwarms() const { return Swarms; }

	/** 미니맵 뷰모델 반환 — Widget이 읽기 전용으로 사용 */
	FORCEINLINE class UMinimapViewModel* GetMinimapViewModel() const { return MinimapViewModelInst; }

protected:
	/** 웨이포인트 수집 — BeginPlay 이후 호출 */
	void CollectWaypoints();

	/** 1. 군집 이동 — 웨이포인트 따라 */
	void MoveSwarms(float DeltaTime);

	/** 2. 군집 vs 군집 전투 */
	void ProcessSwarmCombat(float DeltaTime);

	/** 3. 군집 vs 기지 전투 */
	void ProcessSwarmVsBase(float DeltaTime);

	/** 4. Unfold된 군집 위치를 실체(엔티티/NPC) 평균 위치로 갱신 */
	void UpdateUnfoldedSwarmPositions();

	/** 5. 플레이어 근접 감지 + Unfold/Fold */
	void CheckPlayerProximity();

	/** 군집 → 엔티티/NPC 펼치기 */
	void UnfoldSwarm(int32 Index);

	/** 엔티티/NPC → 군집 접기 */
	void FoldSwarm(int32 Index);

	// ──────────── 웨이브 스폰 ────────────
	/** 웨이브 타이밍에 따라 군집 생성 */
	void TickWaveSpawning(float DeltaTime);

	/** 스포너에서 군집 생성 */
	void SpawnSwarmFromSpawner(class ATPSSwarmSpawner* Spawner);

	/** 빅웨이브 군집 생성 (적 스포너에서) */
	void SpawnBigWave(int32 TotalCount);

	/** 월드에서 SwarmSpawner 수집 */
	void CollectSwarmSpawners();

private:
	TArray<FSwarm> Swarms;
	int32 NextSwarmID = 0;

	/** 미니맵 뷰모델 — Initialize에서 생성, Tick에서 갱신 */
	UPROPERTY()
	TObjectPtr<class UMinimapViewModel> MinimapViewModelInst;

	/** 캐싱된 웨이포인트 세트 — 각 링크드리스트 체인이 하나의 Route */
	TArray<TArray<FVector>> EnemyWaypointRoutes;
	float EnemyWaypointAcceptRadius = 5000.f;

	TArray<TArray<FVector>> AllyWaypointRoutes;
	float AllyWaypointAcceptRadius = 5000.f;

	/** Unfold/Fold 반경 */
	float UnfoldRadius = 15000.f;
	float FoldRadius = 20000.f;

	/** 군집끼리 교전 판정 반경 */
	float EngageRadius = 5000.f;

	/** 군집이 기지에 도달하는 판정 반경 */
	float BaseAttackRadius = 2000.f;

	bool bWaypointsCollected = false;

	/** 맵 범위 — 웨이포인트/기지/스포너에서 자동 계산 */
	FVector2D AutoMapCenter = FVector2D::ZeroVector;
	float AutoMapExtent = 50000.f;
	void CalcMapBounds();

	// ──────────── 웨이브 상태 ────────────
	UPROPERTY()
	TObjectPtr<const class UTPSWaveSettings> CachedSettings = nullptr;

	UPROPERTY()
	TObjectPtr<class UTPSEnemyTypeDataAsset> CachedEnemyType = nullptr;

	/** 수집된 스포너 목록 */
	TArray<TWeakObjectPtr<class ATPSSwarmSpawner>> CachedSpawners;

	EWavePhase CurrentPhase = EWavePhase::Idle;
	int32 CurrentWaveLevel = 0;
	float ElapsedTime = 0.f;
	float LastBigWaveTime = 0.f;
	uint8 bIsActive : 1 = false;
};
