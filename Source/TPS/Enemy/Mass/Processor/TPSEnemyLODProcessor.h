#pragma once
#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "TPSEnemyLODProcessor.generated.h"

/**
 * 적 LOD 전환 Processor (GameThread)
 * - DistanceToPlayer 기반 LOD 단계 전환
 * - FullActor: Actor 풀 Get/Return
 * - ISM: Phase 5에서 구현
 * - 히스테리시스 200cm으로 LOD 떨림 방지
 * - 프레임당 전환 상한 20마리로 스파이크 방지
 */
UCLASS()
class TPS_API UTPSEnemyLODProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UTPSEnemyLODProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;

	/** LOD 전환 거리 (cm) — 2km×2km 맵 기준 */
	static constexpr float FullActorDistance = 8000.f;
	static constexpr float ISMDistance = 50000.f;
	static constexpr float Hysteresis = 500.f;

	/** 프레임당 최대 LOD 전환 횟수 (스파이크 방지) */
	static constexpr int32 MaxTransitionsPerFrame = 20;
};
