#include "ECS/Renderer/AEnemyRenderActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

AEnemyRenderActor::AEnemyRenderActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 빈 SceneComponent를 루트로 사용 — ISM은 InitializeISMs에서 동적 생성
	USceneComponent* pRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(pRoot);
}

void AEnemyRenderActor::InitializeISMs(UStaticMesh* Meshes[], int32 Count)
{
	if (ISMComponents.Num() > 0) { return; }

	const int32 LODCount = FMath::Min(Count, HISM_LOD_COUNT);
	ISMComponents.SetNum(HISM_LOD_COUNT);

	for (int32 i = 0; i < LODCount; ++i)
	{
		if (!Meshes[i]) { continue; }

		const FName Name = *FString::Printf(TEXT("ISM_LOD%d"), i);
		auto* pISM = NewObject<UInstancedStaticMeshComponent>(this, Name);

		// ① 메시 먼저 세팅 (RegisterComponent 전) — Mass Entity 패턴
		pISM->SetStaticMesh(Meshes[i]);

		// ② 설정
		pISM->SetMobility(EComponentMobility::Movable);
		pISM->NumCustomDataFloats = 2;
		pISM->SetCanEverAffectNavigation(false);
		pISM->SetReceivesDecals(false);
		pISM->SetCastShadow(false);
		pISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		pISM->SetCollisionProfileName(TEXT("BlockAll"));
		pISM->SetCullDistances(0, 1000000);
		pISM->SetupAttachment(GetRootComponent());

		// ③ 마지막에 RegisterComponent
		pISM->RegisterComponent();

		ISMComponents[i] = pISM;

		UE_LOG(LogTemp, Log, TEXT("[RenderActor] ISM[%d] created: Mesh=%s, Registered=%d"),
			i, *Meshes[i]->GetName(), pISM->IsRegistered());
	}
}

UInstancedStaticMeshComponent* AEnemyRenderActor::GetISMComponent(int32 LODIndex) const
{
	if (ISMComponents.IsValidIndex(LODIndex))
	{
		return ISMComponents[LODIndex].Get();
	}
	return nullptr;
}
