#pragma once

#include "CoreMinimal.h"
#include "Actor/Base/TPSDestroyableBase.h"
#include "TPSAllyBase.generated.h"

/**
 * 아군 기지
 * - 적 AI의 기본 진격 목표
 * - 적 웨이포인트 첫 노드 연결
 */
UCLASS()
class TPS_API ATPSAllyBase : public ATPSDestroyableBase
{
	GENERATED_BODY()

public:
	ATPSAllyBase();

	/** 이 기지로 향하는 적 웨이포인트 시작 노드 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Waypoint")
	TObjectPtr<class ATPSWaypointActor> EnemyWaypointStart = nullptr;

protected:
	virtual void BeginPlay() override;
	virtual void OnBaseDestroyed() override;
};
