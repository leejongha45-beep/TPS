#pragma once
#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "TPSEnemyTransformSyncProcessor.generated.h"

/**
 * Transform 동기화 Processor (GameThread)
 * - MovementFragment.CurrentLocation → Actor/ISM 위치 동기화
 * - FullActor: SetActorLocation + SetActorRotation + SyncAIState
 * - ISM: ISMSubsystem.UpdateInstanceTransform
 * - LODProcessor 이후, MeleeProcessor 이전 실행
 */
UCLASS()
class TPS_API UTPSEnemyTransformSyncProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UTPSEnemyTransformSyncProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
