#include "ECS/Scheduler/EnemyManagerSubsystem.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "ECS/Component/Components.h"
#include "ECS/Renderer/AEnemyRenderActor.h"
#include "ECS/Scheduler/EnemyScheduler.h"
#include "ECS/System/SpawnSystem.h"
#include "Engine/World.h"

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
			EnemySchedulerInst->PreTickCallback = [this]() { FlushSpawnQueue(); };
		}
	}

	// ② 렌더 액터 스폰 + 스케줄러에 HISM 연결
	UWorld* pWorld = GetWorld();
	if (ensure(pWorld))
	{
		if (!RenderActorInst)
		{
			RenderActorInst = pWorld->SpawnActor<AEnemyRenderActor>();
			if (ensure(RenderActorInst.Get()))
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
	if (ensure(RenderActorInst.Get()))
	{
		RenderActorInst->Destroy();
		RenderActorInst = nullptr;
	}

	SpawnQueue.Empty();

	Super::Deinitialize();
}

void UEnemyManagerSubsystem::StartWave(int32 WaveNumber)
{
	if (!ensure(EnemySchedulerInst)) { return; }

	CurrentWave = WaveNumber;
	const int32 EnemyCount = FMath::Min(20 + (WaveNumber * 10), 3000);

	// ① QueueSpawn으로 일괄 등록
	for (int32 i = 0; i < EnemyCount; ++i)
	{
		FEnemySpawnParams Params;
		Params.Position = FVector(FMath::RandRange(-2000.f, 2000.f),
		                          FMath::RandRange(-2000.f, 2000.f), 0.f);
		Params.MaxHealth      = 50.f;
		Params.MaxSpeed       = 300.f;
		Params.AttackDamage   = ECSConstants::AttackDamage;
		Params.AttackCooldown = ECSConstants::AttackCooldown;

		QueueSpawn(Params);
	}

	// ② 즉시 처리
	FlushSpawnQueue();
}

void UEnemyManagerSubsystem::QueueSpawn(const FEnemySpawnParams& Params)
{
	SpawnQueue.Add(Params);
}

void UEnemyManagerSubsystem::FlushSpawnQueue()
{
	if (!ensure(EnemySchedulerInst)) { return; }

	UHierarchicalInstancedStaticMeshComponent* pHISM = GetHISM();
	if (!ensure(pHISM)) { return; }

	const int32 QueueCount = SpawnQueue.Num();
	for (int32 i = 0; i < QueueCount; ++i)
	{
		const FEnemySpawnParams& Params = SpawnQueue[i];

		// ① ECS Entity 생성
		entt::entity Entity = SpawnSystem::Spawn(EnemySchedulerInst->GetRegistry(), Params);

		// ② HISM 인스턴스 등록 → CRenderProxy 세팅
		const FTransform InstanceTransform(FQuat::Identity, Params.Position);
		const int32 InstanceIndex = pHISM->AddInstance(InstanceTransform, true);

		EnemySchedulerInst->GetRegistry().get<CRenderProxy>(Entity).InstanceIndex = InstanceIndex;
		EnemySchedulerInst->GetRegistry().get<CRenderProxyPrev>(Entity).InstanceIndex = InstanceIndex;

		// ③ 역방향 룩업 테이블 등록
		EnemySchedulerInst->GetInstanceToEntity().Add(Entity);
	}

	SpawnQueue.Reset();
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
	if (ensure(RenderActorInst.Get()))
	{
		return RenderActorInst->GetHISMComponent();
	}
	return nullptr;
}
