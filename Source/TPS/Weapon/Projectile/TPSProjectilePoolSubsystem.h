#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TPSProjectilePoolSubsystem.generated.h"

/**
 * 발사체 오브젝트 풀 서브시스템
 * - WorldSubsystem — 레벨 단위 생명주기
 * - Initialize: Config DataAsset 로드 + 발사체 클래스 동기 로드
 * - OnWorldBeginPlay: 초기 배치 스폰 + 지연 스폰 타이머 시작
 * - Get/Return 패턴으로 발사체 재활용
 */
UCLASS()
class TPS_API UTPSProjectilePoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/** 풀에서 비활성 발사체 1개 가져오기 */
	class ATPSProjectileBase* GetProjectile();

	/** 관통탄 풀에서 1개 가져오기 */
	class ATPSProjectileBase* GetPenetratingProjectile();

	/** 사용 완료된 발사체를 풀에 반환 */
	void ReturnProjectile(class ATPSProjectileBase* InProjectile);

protected:
	/** 발사체 InCount개 즉시 스폰 */
	void SpawnProjectileBatch(int32 InCount);

	/** 타이머 콜백 — 배치 단위로 나머지 스폰 */
	void DeferredSpawn();

	/** 풀 설정 DataAsset (크기, 배치 수 등) */
	UPROPERTY()
	TObjectPtr<class UTPSProjectilePoolConfig> ConfigAsset;

	/** 발사체 풀 배열 */
	UPROPERTY()
	TArray<TObjectPtr<class ATPSProjectileBase>> Pool;

	/** 로드 완료된 발사체 클래스 */
	UPROPERTY()
	TSubclassOf<class ATPSProjectileBase> LoadedProjectileClass;

	FTimerHandle DeferredSpawnTimerHandle;

	int32 PoolSize = 0;
	int32 DeferredSpawnBatchSize = 0;
	int32 TotalSpawnedCount = 0;

	/** 관통탄 풀 */
	UPROPERTY()
	TArray<TObjectPtr<class ATPSProjectileBase>> PenetratingPool;

	static constexpr int32 PenetratingPoolSize = 20;
};