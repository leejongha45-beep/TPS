// TPSWaypointActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSWaypointActor.generated.h"

/**
 * 적 Rush 경로용 웨이포인트.
 * 에디터에서 NextWaypoint를 드래그하여 링크드 리스트로 연결.
 * NextWaypoint가 nullptr이면 마지막 (기지).
 */
UCLASS(Blueprintable)
class TPS_API ATPSWaypointActor : public AActor
{
	GENERATED_BODY()

public:
	ATPSWaypointActor();

	/** 다음 웨이포인트 — nullptr이면 마지막 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Waypoint")
	TObjectPtr<class ATPSWaypointActor> NextWaypoint = nullptr;

	/** 도착 판정 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint", meta = (ClampMin = "100.0"))
	float AcceptRadius = 500.f;

protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "Waypoint")
	TObjectPtr<class UBillboardComponent> SpriteComponent = nullptr;
#endif
};
