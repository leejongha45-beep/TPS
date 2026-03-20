#include "ECS/Scheduler/EnemyManagerSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ECS/Component/Components.h"
#include "ECS/Data/TPSEnemyTypeDataAsset.h"
#include "ECS/Renderer/AEnemyRenderActor.h"
#include "ECS/Scheduler/EnemyScheduler.h"
#include "ECS/System/SpawnSystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"
#include "Wave/TPSWaveSettings.h"

UEnemyManagerSubsystem::~UEnemyManagerSubsystem()
{
}

void UEnemyManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 스케줄러 생성
	if (!EnemySchedulerInst)
	{
		EnemySchedulerInst = MakeUnique<FEnemyScheduler>();
		if (ensure(EnemySchedulerInst))
		{
			EnemySchedulerInst->Initialize(GetWorld());
			EnemySchedulerInst->PreTickCallback = [this]() { FlushSpawnQueue(); };
		}
	}
}

void UEnemyManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// 렌더 액터 스폰 (BP 클래스) + LOD별 ISM 초기화
	// — Initialize 시점에는 렌더 씬이 미완성이라 ISM이 렌더 큐에 등록 안 됨
	if (RenderActorInst) { return; }

	const UTPSWaveSettings* pSettings = GetDefault<UTPSWaveSettings>();
	UClass* pActorClass = nullptr;
	if (ensure(pSettings) && !pSettings->RenderActorClass.IsNull())
	{
		pActorClass = pSettings->RenderActorClass.LoadSynchronous();
	}
	if (!pActorClass)
	{
		pActorClass = AEnemyRenderActor::StaticClass();
		UE_LOG(LogTemp, Warning, TEXT("[EnemyMgr] RenderActorClass not set — falling back to C++ class"));
	}

	RenderActorInst = InWorld.SpawnActor<AEnemyRenderActor>(pActorClass);
	if (ensure(RenderActorInst.Get()))
	{
		// DataAsset에서 LOD 메시 로드
		UStaticMesh* LODMeshes[HISM_LOD_COUNT] = {};

		if (ensure(pSettings) && !pSettings->EnemyType.IsNull())
		{
			class UTPSEnemyTypeDataAsset* pFirstType = pSettings->EnemyType.LoadSynchronous();
			if (ensure(pFirstType))
			{
				const int32 MeshCount = FMath::Min(pFirstType->LODMeshes.Num(), HISM_LOD_COUNT);
				for (int32 i = 0; i < MeshCount; ++i)
				{
					if (!pFirstType->LODMeshes[i].IsNull())
					{
						LODMeshes[i] = pFirstType->LODMeshes[i].LoadSynchronous();
					}
				}
			}
		}

		// NewObject → SetStaticMesh → 설정 → RegisterComponent (Mass Entity 순서)
		RenderActorInst->InitializeISMs(LODMeshes, HISM_LOD_COUNT);

		// 스케줄러에 LOD별 ISM 참조 전달
		UInstancedStaticMeshComponent* ISMPtrs[HISM_LOD_COUNT] = {};
		for (int32 i = 0; i < HISM_LOD_COUNT; ++i)
		{
			ISMPtrs[i] = RenderActorInst->GetISMComponent(i);
		}
		EnemySchedulerInst->SetHISMs(ISMPtrs, HISM_LOD_COUNT);

		// 지형 캐시 + 웨이포인트 빌드
		// TPSTargetSubsystem에서 기지 위치 자동 가져오기 (ATPSAllyBase::BeginPlay에서 등록됨)
		FVector BaseLocation = FVector::ZeroVector;
		if (UTPSTargetSubsystem* TargetSub = InWorld.GetSubsystem<UTPSTargetSubsystem>())
		{
			BaseLocation = TargetSub->GetAllyBaseLocation();
		}
		EnemySchedulerInst->SetBaseLocation(BaseLocation);

		// 기지 위치가 유효하면 즉시 빌드, 아니면 Lazy 초기화에 위임
		if (!BaseLocation.IsNearlyZero())
		{
			EnemySchedulerInst->BuildTerrainCache(&InWorld, BaseLocation);
			EnemySchedulerInst->CollectWaypoints(&InWorld);
			UE_LOG(LogTemp, Log, TEXT("[EnemyMgr] TerrainCache + Waypoints built at BaseLocation: %s"), *BaseLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemyMgr] BaseLocation is ZeroVector — TerrainCache deferred to lazy init"));
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

	FlushSpawnQueue();
}

void UEnemyManagerSubsystem::QueueSpawn(const FEnemySpawnParams& Params)
{
	SpawnQueue.Add(Params);
}

void UEnemyManagerSubsystem::FlushSpawnQueue()
{
	if (!ensure(EnemySchedulerInst)) { return; }

	// Lazy 지형 캐시 빌드 — 기지 BeginPlay가 OnWorldBeginPlay보다 늦을 때 대비
	if (!EnemySchedulerInst->bTerrainCacheBuilt)
	{
		UWorld* World = GetWorld();
		if (UTPSTargetSubsystem* TargetSub = World ? World->GetSubsystem<UTPSTargetSubsystem>() : nullptr)
		{
			const FVector BaseLocation = TargetSub->GetAllyBaseLocation();
			if (!BaseLocation.IsNearlyZero())
			{
				EnemySchedulerInst->SetBaseLocation(BaseLocation);
				EnemySchedulerInst->BuildTerrainCache(World, BaseLocation);
				EnemySchedulerInst->CollectWaypoints(World);
				UE_LOG(LogTemp, Warning, TEXT("[EnemyMgr] TerrainCache + Waypoints lazy-built at BaseLocation: %s"), *BaseLocation.ToString());
			}
		}
	}

	// 신규 스폰은 항상 Near(LOD0) HISM에 등록
	constexpr int32 SpawnLOD = 0;
	class UInstancedStaticMeshComponent* pHISM = GetHISM(SpawnLOD);
	if (!pHISM) { return; }   // OnWorldBeginPlay 전이면 ISM 미생성 — 조용히 스킵

	const int32 QueueCount = SpawnQueue.Num();
	if (QueueCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyMgr] FlushSpawnQueue — QueueCount=%d"), QueueCount);
	}
	for (int32 i = 0; i < QueueCount; ++i)
	{
		const FEnemySpawnParams& Params = SpawnQueue[i];
		UE_LOG(LogTemp, Log, TEXT("[EnemyMgr] Spawning[%d] Pos=(%.0f, %.0f, %.0f)"),
			i, Params.Position.X, Params.Position.Y, Params.Position.Z);

		// ① ECS Entity 생성
		entt::entity Entity = SpawnSystem::Spawn(EnemySchedulerInst->GetRegistry(), Params);

		// ② HISM 인스턴스 등록 → CRenderProxy 세팅
		const FTransform InstanceTransform(FQuat::Identity, Params.Position);
		const int32 InstanceIndex = pHISM->AddInstance(InstanceTransform, true);

		auto& Proxy = EnemySchedulerInst->GetRegistry().get<CRenderProxy>(Entity);
		Proxy.InstanceIndex = InstanceIndex;
		Proxy.LODLevel = SpawnLOD;

		auto& ProxyPrev = EnemySchedulerInst->GetRegistry().get<CRenderProxyPrev>(Entity);
		ProxyPrev.InstanceIndex = InstanceIndex;
		ProxyPrev.LODLevel = SpawnLOD;

		// ③ 역방향 룩업 테이블 등록
		EnemySchedulerInst->GetInstanceToEntity(SpawnLOD).Add(Entity);
	}

	if (QueueCount > 0)
	{
		EnemySchedulerInst->bHasEntities = true;
		pHISM->MarkRenderStateDirty();
		int32 TotalEntities = 0;
		for (int32 i = 0; i < HISM_LOD_COUNT; ++i)
		{
			TotalEntities += EnemySchedulerInst->GetInstanceToEntity(i).Num();
		}
		UE_LOG(LogTemp, Warning, TEXT("[EnemyMgr] After flush — TotalEntities=%d, LOD0=%d, LOD1=%d, LOD2=%d"),
			TotalEntities,
			EnemySchedulerInst->GetInstanceToEntity(0).Num(),
			EnemySchedulerInst->GetInstanceToEntity(1).Num(),
			EnemySchedulerInst->GetInstanceToEntity(2).Num());
	}

	SpawnQueue.Reset();
}

void UEnemyManagerSubsystem::ApplyDamage(int32 InstanceIndex, uint8 LODLevel, float Damage)
{
	if (ensure(EnemySchedulerInst))
	{
		EnemySchedulerInst->QueueDamage(InstanceIndex, LODLevel, Damage);
	}
}

UInstancedStaticMeshComponent* UEnemyManagerSubsystem::GetHISM(int32 LODIndex) const
{
	if (!RenderActorInst.Get()) { return nullptr; }   // OnWorldBeginPlay 전이면 미생성
	return RenderActorInst->GetISMComponent(LODIndex);
}
