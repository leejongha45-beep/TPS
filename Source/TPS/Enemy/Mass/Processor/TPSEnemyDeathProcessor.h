#pragma once
#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "TPSEnemyDeathProcessor.generated.h"

/**
 * 사망 처리 Processor (GameThread)
 * - Die 상태 Entity 감지
 * - FullActor면 Actor 비활성화 + 풀 반환
 * - Entity 파괴 (Mass Entity 제거)
 */
UCLASS()
class TPS_API UTPSEnemyDeathProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UTPSEnemyDeathProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
