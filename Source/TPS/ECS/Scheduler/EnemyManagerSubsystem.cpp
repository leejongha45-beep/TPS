#include "ECS/Scheduler/EnemyManagerSubsystem.h"
#include "ECS/Scheduler/EnemyScheduler.h"
#include "ECS/Renderer/AEnemyRenderActor.h"
#include "ECS/System/SpawnSystem.h"
#include "ECS/Component/Components.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

UEnemyManagerSubsystem::~UEnemyManagerSubsystem()
{
}

void UEnemyManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ① 스케줄러 생성
	if (!EnemySchedulerInst)
	{
		EnemySchedulerInst = MakeUnique<FEnemyScheduler>();
		if (ensure(EnemySchedulerInst))
		{
			EnemySchedulerInst->Initialize();
		}
	}

	// ② 렌더 액터 스폰 + 스케줄러에 HISM 연결
	UWorld* pWorld = GetWorld();
	if (ensure(pWorld))
	{
		if (!RenderActorInst)
		{
			RenderActorInst = pWorld->SpawnActor<AEnemyRenderActor>();
			if (ensure(RenderActorInst))
			{
				EnemySchedulerInst->SetHISM(GetHISM());
			}
		}
	}
}

void UEnemyManagerSubsystem::Deinitialize()
{
	// ① 스케줄러 해제
	if (ensure(EnemySchedulerInst))
	{
		EnemySchedulerInst->Release();
	}
	EnemySchedulerInst.Reset();

	// ② 렌더 액터 파괴
	if (ensure(RenderActorInst))
	{
		RenderActorInst->Destroy();
		RenderActorInst = nullptr;
	}

	Super::Deinitialize();
}

void UEnemyManagerSubsystem::StartWave(int32 WaveNumber)
{
	if (!ensure(EnemySchedulerInst)) { return; }

	CurrentWave = WaveNumber;
	const int32 EnemyCount = 20 + (WaveNumber * 10);

	UHierarchicalInstancedStaticMeshComponent* pHISM = GetHISM();
	if (!ensure(pHISM)) { return; }

	for (int32 i = 0; i < EnemyCount; ++i)
	{
		// ① ECS Entity 생성
		const FVector SpawnPosition(FMath::RandRange(-2000.f, 2000.f), FMath::RandRange(-2000.f, 2000.f), 0.f);
		entt::entity Entity = SpawnSystem::Spawn(EnemySchedulerInst->GetRegistry(), SpawnPosition, 50.f, 300.f);

		// ② HISM 인스턴스 등록 → CRenderProxy 세팅
		const FTransform InstanceTransform(FQuat::Identity, SpawnPosition);
		const int32 InstanceIndex = pHISM->AddInstance(InstanceTransform, true);

		EnemySchedulerInst->GetRegistry().get<CRenderProxy>(Entity).InstanceIndex = InstanceIndex;
		EnemySchedulerInst->GetRegistry().get<CRenderProxyPrev>(Entity).InstanceIndex = InstanceIndex;

		// ③ 역방향 룩업 테이블 등록
		EnemySchedulerInst->GetInstanceToEntity().Add(Entity);
	}
}

void UEnemyManagerSubsystem::ApplyDamage(int32 InstanceIndex, float Damage)
{
	if (ensure(EnemySchedulerInst))
	{
		EnemySchedulerInst->QueueDamage(InstanceIndex, Damage);
	}
}

UHierarchicalInstancedStaticMeshComponent* UEnemyManagerSubsystem::GetHISM() const
{
	if (ensure(RenderActorInst))
	{
		return RenderActorInst->HISMComponent;
	}
	return nullptr;
}