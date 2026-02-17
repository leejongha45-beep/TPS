#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TPSProjectilePoolConfig.generated.h"

/**
 * 발사체 풀 설정 DataAsset
 * - TPSProjectilePoolSubsystem이 참조
 * - 풀 크기, 초기 스폰 수, 지연 스폰 배치 크기 설정
 * - SoftClassPtr로 발사체 클래스 비동기 로드 지원
 */
UCLASS()
class TPS_API UTPSProjectilePoolConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 풀링할 발사체 클래스 (SoftPtr — 비동기 로드) */
	UPROPERTY(EditDefaultsOnly, Category = "Pool")
	TSoftClassPtr<class ATPSProjectileBase> ProjectileClassPath;

	/** 풀 최대 크기 */
	UPROPERTY(EditDefaultsOnly, Category = "Pool", meta = (ClampMin = "1"))
	int32 PoolSize = 1000;

	/** 월드 시작 시 즉시 스폰할 수 */
	UPROPERTY(EditDefaultsOnly, Category = "Pool", meta = (ClampMin = "1"))
	int32 InitialSpawnCount = 500;

	/** 지연 스폰 1회당 배치 크기 */
	UPROPERTY(EditDefaultsOnly, Category = "Pool", meta = (ClampMin = "1"))
	int32 DeferredSpawnBatchSize = 10;
};
