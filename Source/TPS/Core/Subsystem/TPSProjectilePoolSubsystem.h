#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TPSProjectilePoolSubsystem.generated.h"

UCLASS()
class TPS_API UTPSProjectilePoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	class ATPSProjectileBase* GetProjectile();
	void ReturnProjectile(class ATPSProjectileBase* InProjectile);

protected:
	void SpawnProjectileBatch(int32 InCount);
	void DeferredSpawn();

	UPROPERTY()
	TObjectPtr<class UTPSProjectilePoolConfig> ConfigAsset;

	UPROPERTY()
	TArray<TObjectPtr<class ATPSProjectileBase>> Pool;

	UPROPERTY()
	TSubclassOf<class ATPSProjectileBase> LoadedProjectileClass;

	FTimerHandle DeferredSpawnTimerHandle;

	int32 PoolSize = 0;
	int32 DeferredSpawnBatchSize = 0;
	int32 TotalSpawnedCount = 0;
};