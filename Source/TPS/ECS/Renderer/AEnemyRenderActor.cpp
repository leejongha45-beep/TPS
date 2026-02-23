#include "ECS/Renderer/AEnemyRenderActor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

AEnemyRenderActor::AEnemyRenderActor()
{
	if (!HISMComponentInst)
	{
		HISMComponentInst = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HISM"));
		if (ensure(HISMComponentInst.Get()))
		{
			RootComponent = HISMComponentInst.Get();

			// 프로젝타일 물리 충돌 대응 (라인 트레이스 불필요)
			HISMComponentInst->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			HISMComponentInst->SetCollisionProfileName(TEXT("BlockAll"));
		}
	}
}