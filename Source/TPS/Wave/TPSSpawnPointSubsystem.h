#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TPSSpawnPointSubsystem.generated.h"

class ATPSEnemySpawnPoint;

/** 스폰 포인트 비활성화 통보 (비활성화된 포인트 수, 비활성화 전 활성 포인트 수) */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSpawnPointsDeactivated, int32, int32);

/**
 * 스폰 포인트 관리 월드 서브시스템
 * - BeginPlay 시 레벨의 모든 ATPSEnemySpawnPoint 캐싱
 * - 아군 기지 기준 거리순 정렬 (가까운 → 먼)
 * - 거리 기반 / 본진 / 랜덤 쿼리 API 제공
 * - SnapToGround 유틸리티 (Z축 라인트레이스)
 * - 기지 파괴 시 비활성화 API
 * - 최종 단계 시 본진 포인트 포함 전환
 *
 * 쿼리 규칙: 모든 쿼리는 활성(bIsActive == true) 포인트만 반환
 * 평소: bIsEnemyHQ 포인트 제외
 * 최종 단계(bFinalPhaseActive): bIsEnemyHQ 포인트 포함
 */
UCLASS()
class TPS_API UTPSSpawnPointSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ UWorldSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	//~ End UWorldSubsystem

	// ──────────── 거리 기반 쿼리 (핵심) ────────────

	/**
	 * 아군 기지에서 가까운 순으로 활성 포인트 N개 반환
	 * @param Count  선택할 포인트 수 (활성 수보다 크면 전체 반환)
	 */
	TArray<ATPSEnemySpawnPoint*> GetSpawnPointsByDistance(int32 Count) const;

	/**
	 * 아군 기지 기준 거리 범위 내 활성 포인트 반환
	 * @param MinDist  최소 거리 (cm)
	 * @param MaxDist  최대 거리 (cm)
	 */
	TArray<ATPSEnemySpawnPoint*> GetSpawnPointsByDistanceRange(float MinDist, float MaxDist) const;

	// ──────────── 특수 쿼리 ────────────

	/** 적 본진(bIsEnemyHQ == true) 활성 포인트만 반환 */
	TArray<ATPSEnemySpawnPoint*> GetEnemyHQSpawnPoints() const;

	/** 전체 활성 포인트에서 중복 없이 랜덤 N개 반환 */
	TArray<ATPSEnemySpawnPoint*> GetRandomSpawnPoints(int32 Count) const;

	// ──────────── 비활성화 API ────────────

	/** 전달받은 포인트들 일괄 비활성화 (기지 파괴 시 호출) */
	void DeactivateSpawnPoints(const TArray<ATPSEnemySpawnPoint*>& Points);

	/** 스폰 포인트 비활성화 통보 델리게이트 */
	FOnSpawnPointsDeactivated OnSpawnPointsDeactivatedDelegate;

	// ──────────── 최종 단계 API ────────────

	/** 최종 단계 진입 — 이후 쿼리에서 본진 포인트도 포함 */
	void ActivateFinalPhase();

	FORCEINLINE bool IsFinalPhaseActive() const { return bFinalPhaseActive; }

	// ──────────── 유틸리티 ────────────

	/**
	 * Z축 라인트레이스로 바닥 위치 보정
	 * @param InLocation   보정할 위치
	 * @param TraceHeight  트레이스 높이 (위/아래 양방향, cm)
	 * @return 바닥 위치 (실패 시 원래 위치)
	 */
	FVector SnapToGround(const FVector& InLocation, float TraceHeight = 5000.f) const;

	/** 아군 기지 위치 수동 설정 */
	void SetAllyBaseLocation(const FVector& Location);

	FORCEINLINE FVector GetAllyBaseLocation() const { return AllyBaseLocation; }

protected:
	/** 아군 기지 기준 거리순 정렬된 스폰 포인트 캐시 (가까운 → 먼) */
	UPROPERTY()
	TArray<ATPSEnemySpawnPoint*> SortedSpawnPoints;

	/** 아군 기지 위치 (거리 계산 기준점) */
	FVector AllyBaseLocation = FVector::ZeroVector;

	/** 최종 단계 활성화 여부 — true면 쿼리에 본진 포인트 포함 */
	uint8 bFinalPhaseActive : 1 = false;

private:
	/** 모든 SpawnPoint 수집 후 아군 기지 기준 거리순 정렬 */
	void CollectAndSortSpawnPoints(UWorld* World);
};
