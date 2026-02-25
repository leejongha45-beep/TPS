#include "ECS/Scheduler/EnemyScheduler.h"
#include "Async/TaskGraphInterfaces.h"
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

/**
 * ECS 파이프라인 — 멀티스레드 실행
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ [GameThread] Phase 0. PushToPrev_RenderProxy                          │
 * │   Cleanup이 갱신한 CRenderProxy.Current → Prev 반영                    │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ [GameThread] Phase 1. UObject 접근                                    │
 * │   PlayerPosition, HISM 등 UObject를 지역변수로 캐싱                     │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ [GameThread→Worker] Phase 1.5. LODSystem (ParallelFor)                │
 * │   AccumDT 리셋/누적, 거리→LOD Level, bShouldTick 결정                  │
 * │   W: CLOD → PushToPrev → CLODPrev                                    │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ [GameThread] Phase 2. Damage (순차 — 같은 Entity 다중 히트)            │
 * │   R: DamageQueue, InstanceToEntity   W: CHealth   PushToPrev          │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ [GameThread→Worker] Phase 3. AI (ParallelFor) — LOD SKIP              │
 * │   R: Prev 컴포넌트, CLODPrev   W: CEnemyState, CMovement   PushToPrev │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ [GameThread] Phase 3.1. Attack (순차 — 3단계 상태 머신 + 데미지 집계) │
 * │   R: CAttackPrev, CEnemyStatePrev, CAnimationPrev                     │
 * │   W: CAttack, CEnemyState → PushToPrev                               │
 * │   UObject: IDamageable::ReceiveDamage (프레임당 1회 집계 호출)         │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ [WorkerThread] Phase 3.5+4. Separation ∥ Death (TaskGraph 병렬)       │
 * │   Separation: R: CTransformPrev, Grid(읽기전용)                       │
 * │               W: CMovement.Velocity (비-Dying Entity) PushToPrev      │
 * │   Death:      R: CEnemyStatePrev, CAnimationPrev                      │
 * │               W: CEnemyState=Dead (Dying Entity만)                    │
 * │   Write 대상 완전 분리 + 엔티티 범위 상호 배타                          │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ ── Barrier: WaitUntilTasksComplete (Separation + Death) ──            │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ [WorkerThread] Phase 5+6. Animation ∥ Movement (TaskGraph 병렬)       │
 * │   — LOD SKIP + AccumDT 보상                                          │
 * │   Animation: W: CAnimation, CAnimationPrev                            │
 * │   Movement:  W: CTransform, CTransformPrev                            │
 * │   Write 대상 완전 분리, CEnemyStatePrev/CLODPrev Read-Only 공유        │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ ── Barrier: WaitUntilTasksComplete ──                                 │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ [GameThread] Phase 7. Visualization — LOD SKIP + 변경 감지            │
 * │   R: CTransformPrev, CAnimationPrev, CRenderProxyPrev, CLODPrev,      │
 * │      CVisCachePrev   W: HISM, CVisCache → PushToPrev                  │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ [GameThread] Phase 8. Cleanup (HISM + Registry.destroy)               │
 * │   Dead Entity 수집 → 내림차순 RemoveInstance → swap 보정 → 파괴        │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ ++FrameCounter (LOD 틱 주기 산정용)                                   │
 * └─────────────────────────────────────────────────────────────────────────┘
 */
void FEnemyScheduler::Tick(float DeltaTime)
{
	UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
	if (!World) { return; }

	// Pre-Tick: 큐잉된 스폰 요청 일괄 처리 (Phase 0 전에 실행)
	if (PreTickCallback) { PreTickCallback(); }

	// 0. PushToPrev_RenderProxy — Cleanup에서 갱신된 CRenderProxy 반영
	{
		auto View = Registry.view<CRenderProxy, CRenderProxyPrev>();
		for (auto Entity : View)
		{
			View.get<CRenderProxyPrev>(Entity).InstanceIndex = View.get<CRenderProxy>(Entity).InstanceIndex;
		}
	}

	// 1. UObject 접근 — 지역변수 캐싱
	const FVector PlayerPosition = GetFrom<FVector>(World, GetPlayerPosition);
	UHierarchicalInstancedStaticMeshComponent* const pHISM = HISMRef;
	IDamageable* pCharacterDamageable = Cast<IDamageable>(UGameplayStatics::GetPlayerPawn(World, 0));

	// 1.5. Phase_LOD (AccumDT 누적/리셋 + bShouldTick 결정)
	LODSystem::Tick(Registry, PlayerPosition, DeltaTime, FrameCounter);

	// 2. Phase_Damage (큐 소비 → CHealth 감산 → PushToPrev)
	DamageSystem::Tick(Registry, DamageQueue, InstanceToEntity);

	// 3. Phase_AI
	AISystem::Tick(Registry, PlayerPosition, AttackRange);

	// 3.1. Phase_Attack (쿨다운 틱 + 데미지 집계 → IDamageable)
	AttackSystem::Tick(Registry, DeltaTime, pCharacterDamageable);

	// 3.5+4. Phase_Separation ∥ Phase_Death ── [WorkerThread] TaskGraph 병렬 실행
	// Separation writes: CMovement (비-Dying Entity)
	// Death writes:      CEnemyState (Dying Entity만)
	// Write 대상 완전 분리 + 엔티티 범위 상호 배타
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
	// Animation writes: CAnimation, CAnimationPrev
	// Movement writes:  CTransform, CTransformPrev
	// Write 대상 완전 분리 — CEnemyStatePrev는 양쪽 Read-Only
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

	// ── Barrier: Phase 5+6 완료 대기 ── [GameThread]
	FTaskGraphInterface::Get().WaitUntilTasksComplete({AnimTask, MoveTask});

	// 7. Phase_Visualization
	if (ensure(pHISM))
	{
		VisualizationSystem::Tick(Registry, pHISM);
	}

	// 8. Phase_Cleanup (HISM 제거 + Entity 파괴)
	if (ensure(pHISM))
	{
		CleanupSystem::Tick(Registry, pHISM, InstanceToEntity);
	}

	++FrameCounter;
}

TStatId FEnemyScheduler::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FEnemyScheduler, STATGROUP_Tickables);
}

void FEnemyScheduler::Initialize()
{
	bIsActive = true;
}

void FEnemyScheduler::Release()
{
	bIsActive = false;
	Registry.clear();
}