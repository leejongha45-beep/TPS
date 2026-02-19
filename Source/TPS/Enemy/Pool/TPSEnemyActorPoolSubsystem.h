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
 * - Pool 고갈 시 긴급 배치 확장 (프레임당 상한 제어)
 */
UCLASS()
class TPS_API UTPSEnemyActorPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/** 풀에서 비활성 적 1마리 가져오기 (Pool 고갈 시 자동 확장, 상한 초과 시 nullptr) */
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

	// ── 긴급 확장 제어 ──

	/** 긴급 확장 1회당 스폰 수 */
	static constexpr int32 ExpansionBatchSize = 10;

	/** 프레임당 긴급 확장 최대 호출 횟수 (10회 × 10마리 = 100마리/프레임) */
	static constexpr int32 MaxExpansionPerFrame = 10;

	/** 현재 프레임 긴급 확장 호출 횟수 (매 프레임 리셋) */
	int32 FrameExpansionCount = 0;

	/** 마지막 확장이 발생한 프레임 번호 */
	uint64 LastExpansionFrameNumber = 0;
};
