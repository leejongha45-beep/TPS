#pragma once
#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "TPSEnemyMeleeProcessor.generated.h"

/**
 * 근접 공격 판정 Processor (GameThread)
 * - Attack 상태 + FullActor + ActorRef 유효 조건에서 ApplyDamage
 * - 쿨다운 기반 반복 공격
 * - 공격 사거리(150cm) < FullActor 거리(5000cm) → 공격 가능 적은 항상 FullActor
 */
UCLASS()
class TPS_API UTPSEnemyMeleeProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UTPSEnemyMeleeProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
