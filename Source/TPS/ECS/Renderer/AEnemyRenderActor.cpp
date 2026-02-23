#include "ECS/Renderer/AEnemyRenderActor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

AEnemyRenderActor::AEnemyRenderActor()
{
	if (!HISMComponent)
	{
		HISMComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HISM"));
		if (ensure(HISMComponent))
		{
			RootComponent = HISMComponent;

			// 프로젝타일 물리 충돌 대응 (라인 트레이스 불필요)
			HISMComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			HISMComponent->SetCollisionProfileName(TEXT("BlockAll"));
		}
	}
}
