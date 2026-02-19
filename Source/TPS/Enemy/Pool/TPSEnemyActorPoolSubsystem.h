#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TPSEnemyActorPoolSubsystem.generated.h"

/**
 * 적 액터 오브젝트 풀 서브시스템
 * - WorldSubsystem — 레벨 단위 생명주기
 * - Initialize: Config DataAsset 로드 + 풀 메모리 예약
 * - OnWorldBeginPlay: 클래스 동기 로드 + 초기 배치 스폰 + 지연 스폰 타이머
 * - Get/Return 패턴으로 적 폰 재활용
 * - TPSProjectilePoolSubsystem과 동일 아키텍처
 */
UCLASS()
class TPS_API UTPSEnemyActorPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/** 풀에서 비활성 적 1마리 가져오기 */
	class ATPSEnemyPawnBase* GetEnemy();

	/** 사용 완료된 적을 풀에 반환 */
	void ReturnEnemy(class ATPSEnemyPawnBase* InEnemy);

protected:
	/** 적 InCount마리 즉시 스폰 */
	void SpawnEnemyBatch(int32 InCount);

	/** 타이머 콜백 — 배치 단위로 나머지 스폰 */
	void DeferredSpawn();

	/** 풀 설정 DataAsset */
	UPROPERTY()
	TObjectPtr<class UTPSEnemyActorPoolConfig> ConfigAsset;

	/** 적 풀 배열 */
	UPROPERTY()
	TArray<TObjectPtr<class ATPSEnemyPawnBase>> Pool;

	/** 로드 완료된 적 폰 클래스 */
	UPROPERTY()
	TSubclassOf<class ATPSEnemyPawnBase> LoadedEnemyClass;

	FTimerHandle DeferredSpawnTimerHandle;

	int32 PoolSize = 0;
	int32 DeferredSpawnBatchSize = 0;
	int32 TotalSpawnedCount = 0;
};
