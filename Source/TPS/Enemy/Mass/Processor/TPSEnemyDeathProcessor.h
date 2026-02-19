#pragma once
#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "TPSEnemyDeathProcessor.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnEnemyKilled);

/**
 * 사망 처리 Processor (GameThread)
 * - Die 상태 Entity 감지
 * - FullActor면 Actor 비활성화 + 풀 반환
 * - ISM이면 인스턴스 제거
 * - 킬 통보 델리게이트 Broadcast
 * - Entity 파괴 (Mass Entity 제거)
 */
UCLASS()
class TPS_API UTPSEnemyDeathProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UTPSEnemyDeathProcessor();

	/** 적 사망 시 Broadcast — WaveManager 등 외부 시스템이 바인딩 */
	static FOnEnemyKilled OnEnemyKilledDelegate;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
