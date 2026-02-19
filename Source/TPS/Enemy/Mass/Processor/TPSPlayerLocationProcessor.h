#pragma once
#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "TPSPlayerLocationProcessor.generated.h"

/**
 * 플레이어 위치 갱신 Processor (GameThread)
 * - 매 프레임 GetPlayerPawn() → SharedFragment에 위치 저장
 * - 다른 워커 Processor(AI, Movement)보다 먼저 실행
 */
UCLASS()
class TPS_API UTPSPlayerLocationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UTPSPlayerLocationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
