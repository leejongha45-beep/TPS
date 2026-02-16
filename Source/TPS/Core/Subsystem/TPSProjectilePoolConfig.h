#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TPSProjectilePoolConfig.generated.h"

UCLASS()
class TPS_API UTPSProjectilePoolConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Pool")
	TSoftClassPtr<class ATPSProjectileBase> ProjectileClassPath;

	UPROPERTY(EditDefaultsOnly, Category = "Pool", meta = (ClampMin = "1"))
	int32 PoolSize = 1000;

	UPROPERTY(EditDefaultsOnly, Category = "Pool", meta = (ClampMin = "1"))
	int32 InitialSpawnCount = 500;

	UPROPERTY(EditDefaultsOnly, Category = "Pool", meta = (ClampMin = "1"))
	int32 DeferredSpawnBatchSize = 10;
};
