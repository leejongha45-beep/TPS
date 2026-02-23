#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ECS/Scheduler/EnemyScheduler.h"
#include "EnemyManagerSubsystem.generated.h"

/**
 * 적 ECS 매니저 서브시스템
 * - FEnemyScheduler 소유 (TUniquePtr)
 * - AEnemyRenderActor(HISM) 소유
 * - StartWave 호출 시 적 일괄 스폰
 */
UCLASS()
class TPS_API UEnemyManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual ~UEnemyManagerSubsystem() override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * 웨이브 시작 — 적 일괄 스폰
	 * @param WaveNumber   현재 웨이브 번호 (적 수 산출에 사용)
	 */
	void StartWave(int32 WaveNumber);

	/** 외부 데미지 적용 — 프로젝타일 OnHit에서 호출
	 *  @param InstanceIndex  HISM 인스턴스 인덱스 (FHitResult.Item)
	 *  @param Damage         데미지량 */
	void ApplyDamage(int32 InstanceIndex, float Damage);

	FORCEINLINE FEnemyScheduler* GetScheduler() const { return EnemySchedulerInst.Get(); }
	class UHierarchicalInstancedStaticMeshComponent* GetHISM() const;

protected:
	TUniquePtr<FEnemyScheduler> EnemySchedulerInst;

	UPROPERTY()
	TObjectPtr<class AEnemyRenderActor> RenderActorInst;

	int32 CurrentWave = 0;
};
