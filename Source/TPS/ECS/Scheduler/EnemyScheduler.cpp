#include "ECS/Scheduler/EnemyScheduler.h"
#include "Async/TaskGraphInterfaces.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ECS/Component/Components.h"
#include "ECS/System/AISystem.h"
#include "ECS/System/AnimationSystem.h"
#include "ECS/System/AttackSystem.h"
#include "ECS/System/CleanupSystem.h"
#include "ECS/System/DamageSystem.h"
#include "ECS/System/DeathSystem.h"
#include "ECS/System/LODSystem.h"
#include "ECS/System/MovementSystem.h"
#include "ECS/System/FlowFieldSystem.h"
#include "ECS/System/SeparationSystem.h"
#include "ECS/System/VisualizationSystem.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/Interface/Data/Damageable.h"
#include "Utils/Template/Getter.h"

FEnemyScheduler::FEnemyScheduler()
{
}

FEnemyScheduler::~FEnemyScheduler()
{
}

FVector GetPlayerPosition(const UWorld* World)
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
	return Player ? Player->GetActorLocation() : FVector::ZeroVector;
}

void FEnemyScheduler::Tick(float DeltaTime)
{
	UWorld* World = CachedWorld.Get();
	if (!World) { return; }

	// 엔티티 없으면 스킵 — 워커 스레드 view 생성 시 entt 풀 race condition 방지
	if (!bHasEntities)
	{
		if (PreTickCallback) { PreTickCallback(); }
		return;
	}

	// Pre-Tick: 큐잉된 스폰 요청 일괄 처리 (Phase 0 전에 실행)
	if (PreTickCallback) { PreTickCallback(); }

	// 0. PushToPrev_RenderProxy — Cleanup/LODTransition에서 갱신된 CRenderProxy 반영
	{
		auto View = Registry.view<CRenderProxy, CRenderProxyPrev>();
		for (auto Entity : View)
		{
			auto& Prev = View.get<CRenderProxyPrev>(Entity);
			const auto& Curr = View.get<CRenderProxy>(Entity);
			Prev.InstanceIndex = Curr.InstanceIndex;
			Prev.LODLevel = Curr.LODLevel;
		}
	}

	// 1. UObject 접근 — 지역변수 캐싱
	const FVector PlayerPosition = GetFrom<FVector>(World, GetPlayerPosition);
	IDamageable* pCharacterDamageable = Cast<IDamageable>(UGameplayStatics::GetPlayerPawn(World, 0));

	// 1.1. Chase 엔티티 NavMesh 경로 쿼리 [GameThread]
	MovementSystem::UpdateChaseTargets(Registry, World, PlayerPosition, FrameCounter);

	// 1.5. Phase_LOD (AccumDT 누적/리셋 + bShouldTick 결정)
	LODSystem::Tick(Registry, PlayerPosition, DeltaTime, FrameCounter);

	// 2. Phase_Damage (큐 소비 → CHealth 감산 → PushToPrev)
	DamageSystem::Tick(Registry, DamageQueue, InstanceToEntityPerLOD);

	// 3. Phase_AI (Rush: FlowField / Chase: NavTarget)
	AISystem::Tick(Registry, PlayerPosition, AttackRange, BaseFlowField, BaseLocation);

	// 3.1. Phase_Attack (쿨다운 틱 + 데미지 집계 → IDamageable)
	AttackSystem::Tick(Registry, DeltaTime, pCharacterDamageable);

	// 3.5+4. Phase_Separation ∥ Phase_Death ── [WorkerThread] TaskGraph 병렬 실행
	FGraphEventRef SepTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
		[&Registry = Registry, &PlayerPosition]()
		{
			SeparationSystem::Tick(Registry, PlayerPosition);
		},
		TStatId{}, nullptr, ENamedThreads::AnyHiPriThreadHiPriTask
	);

	FGraphEventRef DeathTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
		[&Registry = Registry]()
		{
			DeathSystem::Tick(Registry);
		},
		TStatId{}, nullptr, ENamedThreads::AnyHiPriThreadHiPriTask
	);

	// ── Barrier: Separation + Death 완료 대기 ──
	FTaskGraphInterface::Get().WaitUntilTasksComplete({SepTask, DeathTask});

	// 5+6. Phase_Animation ∥ Phase_Movement ── [WorkerThread] TaskGraph 병렬 실행
	FGraphEventRef AnimTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
		[&Registry = Registry, DeltaTime]()
		{
			AnimationSystem::Tick(Registry, DeltaTime);
		},
		TStatId{}, nullptr, ENamedThreads::AnyHiPriThreadHiPriTask
	);

	FGraphEventRef MoveTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
		[&Registry = Registry, DeltaTime, &FlowField = BaseFlowField]()
		{
			MovementSystem::Tick(Registry, DeltaTime, FlowField);
		},
		TStatId{}, nullptr, ENamedThreads::AnyHiPriThreadHiPriTask
	);

	// ── Barrier: Phase 5+6 완료 대기 ── [GameThread]
	FTaskGraphInterface::Get().WaitUntilTasksComplete({AnimTask, MoveTask});

	// 7. LOD Transition — LOD 레벨 변경 시 ISM간 인스턴스 이동
	LODSystem::TransitionInstances(Registry, HISMRefs, InstanceToEntityPerLOD);

	// 7.5. Phase_Visualization — LOD별 HISM 갱신
	for (int32 LODIdx = 0; LODIdx < HISM_LOD_COUNT; ++LODIdx)
	{
		if (HISMRefs[LODIdx])
		{
			VisualizationSystem::Tick(Registry, HISMRefs[LODIdx], static_cast<uint8>(LODIdx));
		}
	}

	// 8. Phase_Cleanup (HISM 제거 + Entity 파괴)
	for (int32 LODIdx = 0; LODIdx < HISM_LOD_COUNT; ++LODIdx)
	{
		if (HISMRefs[LODIdx])
		{
			CleanupSystem::Tick(Registry, HISMRefs[LODIdx],
			                    InstanceToEntityPerLOD[LODIdx], static_cast<uint8>(LODIdx));
		}
	}

	++FrameCounter;
}

TStatId FEnemyScheduler::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FEnemyScheduler, STATGROUP_Tickables);
}

void FEnemyScheduler::Initialize(UWorld* InWorld)
{
	CachedWorld = InWorld;
	bIsActive = true;
	BaseFlowField.Initialize();
}

void FEnemyScheduler::BuildFlowField(class UWorld* World, const FVector& MapCenter)
{
	// Pass 1 [GameThread]: 라인트레이스 → Heights + Blocked
	FlowFieldSystem::BuildTerrainCache(BaseFlowField, World, MapCenter);

	// 기지 셀 인덱스
	const int32 GoalIndex = BaseFlowField.WorldToIndex(BaseLocation.X, BaseLocation.Y);

	// Pass 2+3 [Worker]: 절벽 엣지 + BFS → TaskGraph 디스패치
	FGraphEventRef BuildTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
		[this, GoalIndex]()
		{
			FlowFieldSystem::BuildCliffEdgesAndBFS(BaseFlowField, GoalIndex);
		},
		TStatId{}, nullptr, ENamedThreads::AnyHiPriThreadHiPriTask
	);

	bFlowFieldBuilt = true;

	// Worker 완료 대기 — BeginPlay 시점이라 블로킹 허용
	FTaskGraphInterface::Get().WaitUntilTasksComplete({BuildTask});
}

void FEnemyScheduler::Release()
{
	bIsActive = false;
	Registry.clear();
}

void FEnemyScheduler::SetHISMs(UInstancedStaticMeshComponent* const* InHISMs, int32 Count)
{
	const int32 Num = FMath::Min(Count, HISM_LOD_COUNT);
	for (int32 i = 0; i < Num; ++i)
	{
		HISMRefs[i] = InHISMs[i];
	}
}

int32 FEnemyScheduler::FindLODIndexByHISM(const UInstancedStaticMeshComponent* InHISM) const
{
	for (int32 i = 0; i < HISM_LOD_COUNT; ++i)
	{
		if (HISMRefs[i] == InHISM) { return i; }
	}
	return INDEX_NONE;
}
