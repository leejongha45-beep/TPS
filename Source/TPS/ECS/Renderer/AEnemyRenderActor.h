#pragma once

#include "GameFramework/Actor.h"
#include "AEnemyRenderActor.generated.h"

UCLASS()
class AEnemyRenderActor : public AActor
{
	GENERATED_BODY()

public:
	AEnemyRenderActor();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UHierarchicalInstancedStaticMeshComponent> HISMComponentInst;
};