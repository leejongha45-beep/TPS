#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ECS/Scheduler/EnemyScheduler.h"
#include "ECS/Data/EnemySpawnParams.h"
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerKillECS, int32 /* KillCount */);

#include "EnemyManagerSubsystem.generated.h"

/**
 * 적 ECS 매니저 서브시스템
 * - FEnemyScheduler 소유 (TUniquePtr)
 * - AEnemyRenderActor(LOD별 HISM) 소유
 * - QueueSpawn으로 스폰 요청 큐잉, FlushSpawnQueue로 일괄 처리
 */
UCLASS()
class TPS_API UEnemyManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual ~UEnemyManagerSubsystem() override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	void StartWave(int32 WaveNumber);
	void QueueSpawn(const FEnemySpawnParams& Params);
	void FlushSpawnQueue();

	/** 외부 데미지 적용 — 프로젝타일 OnHit에서 호출
	 *  @param InstanceIndex  HISM 인스턴스 인덱스 (FHitResult.Item)
	 *  @param LODLevel       피격된 HISM의 LOD 인덱스
	 *  @param Damage         데미지량 */
	void ApplyDamage(int32 InstanceIndex, uint8 LODLevel, float Damage, bool bFromPlayer = false);

	/** 플레이어가 ECS 적을 처치했을 때 브로드캐스트 */
	FOnPlayerKillECS OnPlayerKillECSDelegate;

	FORCEINLINE FEnemyScheduler* GetScheduler() const { return EnemySchedulerInst.Get(); }

	/** LOD별 HISM 접근 */
	class UInstancedStaticMeshComponent* GetHISM(int32 LODIndex = 0) const;

protected:
	TUniquePtr<FEnemyScheduler> EnemySchedulerInst;

	UPROPERTY()
	TObjectPtr<class AEnemyRenderActor> RenderActorInst;

	TArray<FEnemySpawnParams> SpawnQueue;
	int32 CurrentWave = 0;
};
