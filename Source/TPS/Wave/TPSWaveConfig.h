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
 * - GameModeBase → WaveManager가 참조
 */
UCLASS()
class TPS_API UTPSWaveConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 웨이브 목록 (순서대로 진행) */
	UPROPERTY(EditDefaultsOnly, Category = "Wave")
	TArray<FTPSWaveEntry> Waves;
};
