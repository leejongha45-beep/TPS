#pragma once
#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "TPSEnemyMovementProcessor.generated.h"

/**
 * 적 이동 Processor (WorkerThread)
 * - Chase 상태인 Entity의 CurrentLocation을 TargetLocation 방향으로 갱신
 * - AIProcessor 이후 실행 (TargetLocation 설정 후)
 * - 순수 FVector 연산만 — UObject 접근 없음
 */
UCLASS()
class TPS_API UTPSEnemyMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UTPSEnemyMovementProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
