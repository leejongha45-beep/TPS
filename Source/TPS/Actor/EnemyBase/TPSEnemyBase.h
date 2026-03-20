#pragma once

#include "CoreMinimal.h"
#include "Actor/Base/TPSDestroyableBase.h"
#include "TPSEnemyBase.generated.h"

/**
 * 적 기지
 * - NPC 솔져의 돌격 목표
 * - NPC 웨이포인트 첫 노드 연결
 * - 파괴 시 NPC 돌격 중지
 */
UCLASS()
class TPS_API ATPSEnemyBase : public ATPSDestroyableBase
{
	GENERATED_BODY()

public:
	ATPSEnemyBase();

	/** 이 기지로 향하는 NPC 웨이포인트 시작 노드 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Waypoint")
	TObjectPtr<class ATPSNPCWaypointActor> NPCWaypointStart = nullptr;

	/** 캐싱된 NPC 웨이포인트 배열 */
	FORCEINLINE const TArray<FVector>& GetCachedWaypoints() const { return CachedWaypoints; }
	FORCEINLINE float GetWaypointAcceptRadius() const { return WaypointAcceptRadius; }

protected:
	virtual void BeginPlay() override;
	virtual void OnBaseDestroyed() override;

private:
	/** BeginPlay에서 링크드리스트 순회 → 배열 캐싱 */
	void CollectWaypoints();

	TArray<FVector> CachedWaypoints;
	float WaypointAcceptRadius = 5000.f;
};
