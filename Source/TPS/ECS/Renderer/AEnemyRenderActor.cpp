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

			// VAT 커스텀 데이터 슬롯: [0] AnimIndex, [1] AnimTime
			HISMComponentInst->NumCustomDataFloats = 2;

			// 800m 이상 인스턴스 컬링 (LOD Far 500m + 여유 300m)
			HISMComponentInst->SetCullDistances(0, 80000);

			// 프로젝타일 물리 충돌 대응 (라인 트레이스 불필요)
			HISMComponentInst->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			HISMComponentInst->SetCollisionProfileName(TEXT("BlockAll"));
		}
	}
}