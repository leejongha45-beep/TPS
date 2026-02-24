#pragma once

#include "GameFramework/Actor.h"
#include "AEnemyRenderActor.generated.h"

/**
 * 적 렌더 액터 — HISM 기반 대량 인스턴스 렌더링
 * - EnemyManagerSubsystem이 소유, Scheduler에 HISM 참조 전달
 */
UCLASS()
class AEnemyRenderActor : public AActor
{
	GENERATED_BODY()

public:
	AEnemyRenderActor();

	FORCEINLINE class UHierarchicalInstancedStaticMeshComponent* GetHISMComponent() const
	{ return HISMComponentInst.Get(); }

protected:
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<class UHierarchicalInstancedStaticMeshComponent> HISMComponentInst;
};