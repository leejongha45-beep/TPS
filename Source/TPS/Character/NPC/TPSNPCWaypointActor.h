#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSNPCWaypointActor.generated.h"

/**
 * NPC 돌격 경로용 웨이포인트.
 * NextWaypoint 링크드리스트로 연결, nullptr이면 마지막.
 */
UCLASS(Blueprintable)
class TPS_API ATPSNPCWaypointActor : public AActor
{
	GENERATED_BODY()

public:
	ATPSNPCWaypointActor();

	/** 다음 웨이포인트 — nullptr이면 마지막 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Waypoint")
	TObjectPtr<class ATPSNPCWaypointActor> NextWaypoint = nullptr;

	/** 경로 세트 인덱스 — 같은 RouteIndex끼리 하나의 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	int32 RouteIndex = 0;

	/** 도착 판정 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint", meta = (ClampMin = "100.0"))
	float AcceptRadius = 5000.f;

protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "Waypoint")
	TObjectPtr<class UBillboardComponent> SpriteComponent = nullptr;
#endif
};
