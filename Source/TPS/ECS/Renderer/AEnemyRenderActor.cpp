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
		}
	}
}
