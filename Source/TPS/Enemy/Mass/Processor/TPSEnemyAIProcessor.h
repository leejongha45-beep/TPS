#pragma once
#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "TPSEnemyAIProcessor.generated.h"

/**
 * 적 AI 상태머신 Processor (WorkerThread)
 * - SharedFragment에서 플레이어 위치 읽기
 * - 거리 계산 + LODFragment에 캐싱
 * - 상태 전환 (Idle→Chase→Attack, Attack→Chase)
 * - 순수 float 연산만 — UObject 접근 없음
 */
UCLASS()
class TPS_API UTPSEnemyAIProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UTPSEnemyAIProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
