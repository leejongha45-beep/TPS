#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TPSWaveConfig.generated.h"

/** 단일 웨이브 정보 */
USTRUCT(BlueprintType)
struct FTPSWaveEntry
{
	GENERATED_BODY()

	/** 이 웨이브에서 스폰할 적 Entity Config */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UMassEntityConfigAsset> EnemyEntityConfig;

	/** 스폰 수 (웨이브당 100마리+) */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"))
	int32 SpawnCount = 100;

	/** 스폰 반경 (플레이어 중심, cm) */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "500"))
	float SpawnRadius = 8000.f;

	/** 웨이브 시작 전 카운트다운 (초) */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0"))
	float CountdownTime = 3.f;

	/**
	 * 사용할 스폰 포인트 수 (아군 기지에서 가까운 순)
	 * 0 = 전체 활성 포인트 사용, 양수 = 가까운 순 N개 선택
	 * 최종 단계 시 본진 포인트도 자동 포함
	 */
	UPROPERTY(EditDefaultsOnly, Category = "SpawnPoint", meta = (ClampMin = "0"))
	int32 SpawnPointCount = 0;
};

/**
 * 웨이브 설정 DataAsset
 * - 에디터에서 웨이브별 적 구성 세팅
 * - Waves 배열을 무한 순환 (Cycle)
 * - 순환할 때마다 SpawnCountMultiplier 누적 적용
 * - GameModeBase → WaveManager가 참조
 */
UCLASS()
class TPS_API UTPSWaveConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 웨이브 목록 (순서대로 진행, 끝나면 처음부터 순환) */
	UPROPERTY(EditDefaultsOnly, Category = "Wave")
	TArray<FTPSWaveEntry> Waves;

	// ──────────── 무한 웨이브 스케일링 ────────────

	/**
	 * 순환(Cycle)마다 스폰 수에 곱하는 배율
	 * Cycle 0 = 1.0, Cycle 1 = 배율^1, Cycle 2 = 배율^2 ...
	 * 예: 1.5면 1회차 100마리 → 2회차 150마리 → 3회차 225마리
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Scaling", meta = (ClampMin = "1.0"))
	float SpawnCountMultiplier = 1.5f;

	/**
	 * 순환당 스폰 수 상한 (한 웨이브 최대치, 퍼포먼스 안전장치)
	 * 0 = 제한 없음
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Scaling", meta = (ClampMin = "0"))
	int32 MaxSpawnCountPerWave = 2000;

	// ──────────── 동시 존재 제한 ────────────

	/**
	 * 월드에 동시에 존재할 수 있는 최대 적 수
	 * 초과분은 대기열에 보류 → 적 사망 시 여유만큼 추가 스폰
	 * 0 = 제한 없음
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Performance", meta = (ClampMin = "0"))
	int32 MaxAliveEnemies = 3000;
};
