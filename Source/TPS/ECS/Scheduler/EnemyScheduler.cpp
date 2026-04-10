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
#include "ECS/System/SeparationSystem.h"
#include "ECS/System/VisualizationSystem.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/Interface/Data/Damageable.h"
#include "Utils/Template/Getter.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"
#include "Wave/TPSWaypointActor.h"

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

DECLARE_STATS_GROUP(TEXT("ECSPhase"), STATGROUP_ECSPhase, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Phase0_PushRenderProxy"), STAT_Phase0_PushRenderProxy, STATGROUP_ECSPhase);
DECLARE_CYCLE_STAT(TEXT("Phase1_ChaseTargets"), STAT_Phase1_ChaseTargets, STATGROUP_ECSPhase);
DECLARE_CYCLE_STAT(TEXT("Phase1_5_LOD"), STAT_Phase1_5_LOD, STATGROUP_ECSPhase);
DECLARE_CYCLE_STAT(TEXT("Phase2_Damage"), STAT_Phase2_Damage, STATGROUP_ECSPhase);
DECLARE_CYCLE_STAT(TEXT("Phase3_AI"), STAT_Phase3_AI, STATGROUP_ECSPhase);
DECLARE_CYCLE_STAT(TEXT("Phase3_1_Attack"), STAT_Phase3_1_Attack, STATGROUP_ECSPhase);
DECLARE_CYCLE_STAT(TEXT("Phase3_5_SepDeath"), STAT_Phase3_5_SepDeath, STATGROUP_ECSPhase);
DECLARE_CYCLE_STAT(TEXT("Phase5_6_AnimMove"), STAT_Phase5_6_AnimMove, STATGROUP_ECSPhase);
DECLARE_CYCLE_STAT(TEXT("Phase7_LODTransition"), STAT_Phase7_LODTransition, STATGROUP_ECSPhase);
DECLARE_CYCLE_STAT(TEXT("Phase7_5_Visualization"), STAT_Phase7_5_Visualization, STATGROUP_ECSPhase);
DECLARE_CYCLE_STAT(TEXT("Phase8_Cleanup"), STAT_Phase8_Cleanup, STATGROUP_ECSPhase);

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
		SCOPE_CYCLE_COUNTER(STAT_Phase0_PushRenderProxy);
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

	// NPC 위치 수집 (Phase 1.1 + Phase 3에서 사용)
	TArray<FVector> NPCPositions;
	if (UTPSTargetSubsystem* TargetSS = World->GetSubsystem<UTPSTargetSubsystem>())
	{
		const auto& NPCs = TargetSS->GetNPCs();
		NPCPositions.Reserve(NPCs.Num());
		for (const auto& NPC : NPCs)
		{
			if (NPC.Get()) { NPCPositions.Add(NPC->GetActorLocation()); }
		}
	}

	// 1.1. Rush/Chase 엔티티 NavMesh 경로 쿼리 [GameThread]
	{
		SCOPE_CYCLE_COUNTER(STAT_Phase1_ChaseTargets);
		MovementSystem::UpdateNavTargets(Registry, World, PlayerPosition, FrameCounter,
		                                 CachedWaypoints, NPCPositions);
	}

	// 1.5. Phase_LOD (AccumDT 누적/리셋 + bShouldTick 결정)
	{
		SCOPE_CYCLE_COUNTER(STAT_Phase1_5_LOD);
		LODSystem::Tick(Registry, PlayerPosition, DeltaTime, FrameCounter);
	}

	// 2. Phase_Damage (큐 소비 → CHealth 감산 → PushToPrev + 히트 이펙트 수집)
	{
		SCOPE_CYCLE_COUNTER(STAT_Phase2_Damage);
		TArray<FHitEffectRequest> HitEffects;
		FramePlayerKillCount = DamageSystem::Tick(Registry, DamageQueue, InstanceToEntityPerLOD, HitEffects);

		if (HitEffects.Num() > 0 && HitEffectCallback)
		{
			HitEffectCallback(HitEffects);
		}
	}

	// 3. Phase_AI (Rush: Waypoint / Chase: NavTarget)
	{
		SCOPE_CYCLE_COUNTER(STAT_Phase3_AI);
		AISystem::Tick(Registry, PlayerPosition, AttackRange,
		               CachedWaypoints, WaypointAcceptRadius, NPCPositions);
	}

	// 3.1. Phase_Attack (쿨다운 틱 + 데미지 집계 → IDamageable)
	{
		SCOPE_CYCLE_COUNTER(STAT_Phase3_1_Attack);
		AttackSystem::Tick(Registry, DeltaTime, pCharacterDamageable);
	}

	// 3.5+4. Phase_Separation ∥ Phase_Death ── [WorkerThread] TaskGraph 병렬 실행
	{
		SCOPE_CYCLE_COUNTER(STAT_Phase3_5_SepDeath);
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

		FTaskGraphInterface::Get().WaitUntilTasksComplete({SepTask, DeathTask});
	}

	// 5+6. Phase_Animation ∥ Phase_Movement ── [WorkerThread] TaskGraph 병렬 실행
	{
		SCOPE_CYCLE_COUNTER(STAT_Phase5_6_AnimMove);
		FGraphEventRef AnimTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
			[&Registry = Registry, DeltaTime]()
			{
				AnimationSystem::Tick(Registry, DeltaTime);
			},
			TStatId{}, nullptr, ENamedThreads::AnyHiPriThreadHiPriTask
		);

		FGraphEventRef MoveTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
			[&Registry = Registry, DeltaTime]()
			{
				MovementSystem::Tick(Registry, DeltaTime);
			},
			TStatId{}, nullptr, ENamedThreads::AnyHiPriThreadHiPriTask
		);

		FTaskGraphInterface::Get().WaitUntilTasksComplete({AnimTask, MoveTask});
	}

	// 7. LOD Transition — LOD 레벨 변경 시 ISM간 인스턴스 이동
	{
		SCOPE_CYCLE_COUNTER(STAT_Phase7_LODTransition);
		LODSystem::TransitionInstances(Registry, HISMRefs, InstanceToEntityPerLOD);
	}

	// 7.5. Phase_Visualization — LOD별 HISM 갱신
	{
		SCOPE_CYCLE_COUNTER(STAT_Phase7_5_Visualization);
		for (int32 LODIdx = 0; LODIdx < HISM_LOD_COUNT; ++LODIdx)
		{
			if (HISMRefs[LODIdx])
			{
				VisualizationSystem::Tick(Registry, HISMRefs[LODIdx], static_cast<uint8>(LODIdx));
			}
		}
	}

	// 8. Phase_Cleanup (HISM 제거 + Entity 파괴)
	{
		SCOPE_CYCLE_COUNTER(STAT_Phase8_Cleanup);
		for (int32 LODIdx = 0; LODIdx < HISM_LOD_COUNT; ++LODIdx)
		{
			if (HISMRefs[LODIdx])
			{
				CleanupSystem::Tick(Registry, HISMRefs[LODIdx],
				                    InstanceToEntityPerLOD[LODIdx], static_cast<uint8>(LODIdx));
			}
		}
	}

	// 킬 콜백 — DamageSystem에서 감지한 플레이어 킬 수 브로드캐스트
	if (FramePlayerKillCount > 0 && PostTickKillCallback)
	{
		PostTickKillCallback(FramePlayerKillCount);
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
}

void FEnemyScheduler::CollectWaypoints(UWorld* World)
{
	if (!World) { return; }

	CachedWaypoints.Reset();

	// 모든 웨이포인트 수집 — Head 찾기 (다른 웨이포인트가 가리키지 않는 것)
	TSet<class ATPSWaypointActor*> AllWaypoints;
	TSet<class ATPSWaypointActor*> ReferencedWaypoints;

	for (TActorIterator<ATPSWaypointActor> It(World); It; ++It)
	{
		class ATPSWaypointActor* WP = *It;
		AllWaypoints.Add(WP);
		if (WP->NextWaypoint)
		{
			ReferencedWaypoints.Add(WP->NextWaypoint);
		}
	}

	// Head = 다른 웨이포인트에 의해 참조되지 않는 것
	class ATPSWaypointActor* Head = nullptr;
	for (class ATPSWaypointActor* WP : AllWaypoints)
	{
		if (!ReferencedWaypoints.Contains(WP))
		{
			Head = WP;
			break;
		}
	}

	if (!Head)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Waypoint] No head waypoint found in level"));
		return;
	}

	// 링크 순회 → 위치 배열 적재
	WaypointAcceptRadius = Head->AcceptRadius;
	class ATPSWaypointActor* Current = Head;
	int32 SafetyCount = 0;
	constexpr int32 MaxWaypoints = 100;

	while (Current && SafetyCount < MaxWaypoints)
	{
		CachedWaypoints.Add(Current->GetActorLocation());
		Current = Current->NextWaypoint;
		++SafetyCount;
	}

	bWaypointsCollected = true;
	UE_LOG(LogTemp, Warning, TEXT("[Waypoint] Collected %d waypoints, AcceptRadius=%.0f"),
		CachedWaypoints.Num(), WaypointAcceptRadius);
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
