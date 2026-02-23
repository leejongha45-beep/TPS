#include "ECS/Scheduler/EnemyScheduler.h"
#include "ECS/Component/Components.h"
#include "ECS/System/DamageSystem.h"
#include "ECS/System/AISystem.h"
#include "ECS/System/DeathSystem.h"
#include "ECS/System/AnimationSystem.h"
#include "ECS/System/MovementSystem.h"
#include "ECS/System/VisualizationSystem.h"
#include "ECS/System/CleanupSystem.h"
#include "Kismet/GameplayStatics.h"
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
 * ECS 파이프라인 — 매 프레임 GameThread에서 순차 실행
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ Phase 0. PushToPrev_RenderProxy                                       │
 * │   Cleanup이 갱신한 CRenderProxy.Current → Prev 반영                    │
 * │   (다른 컴포넌트는 각 시스템 내부에서 PushToPrev 수행)                    │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ Phase 1. UObject 접근                                                 │
 * │   PlayerPosition, HISM 등 UObject를 지역변수로 캐싱                     │
 * │   (이후 Phase는 순수 데이터 연산만 수행)                                 │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ Phase 2. Damage                                                       │
 * │   R: DamageQueue (외부 입력), InstanceToEntity (O(1) 룩업)             │
 * │   W: CHealth.Current -= Damage                                        │
 * │   PushToPrev: CHealth → CHealthPrev                                   │
 * │   Dying/Dead/HP≤0 Entity 스킵                                         │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ Phase 3. AI                                                           │
 * │   R: CTransformPrev, CHealthPrev, CMovementPrev, CEnemyStatePrev      │
 * │   W: CEnemyState (Idle/Moving/AttackReady/Attacking/Dying), CMovement  │
 * │   Dying/Dead Entity는 스킵                                            │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ Phase 4. Death (마킹)                                                 │
 * │   R: CEnemyStatePrev (Dying 필터), CAnimationPrev (AnimTime ≥ Duration)│
 * │   W: CEnemyState = Dead                                               │
 * │   AI와 CEnemyState 쓰기 대상 Entity 상호 배타적                         │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ Phase 5. Animation                                                    │
 * │   R: CAnimationPrev, CEnemyStatePrev                                  │
 * │   W: CAnimation (AnimIndex, AnimTime)                                 │
 * │   상태 전환 시 AnimTime 리셋 — 잔류값 방지                              │
 * │   Dying: 비루핑 클램프 / Dead: 최종 포즈 유지                           │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ Phase 6. Movement                                                     │
 * │   R: CMovementPrev, CTransformPrev, CEnemyStatePrev                   │
 * │   W: CTransform (Position += Velocity * DeltaTime)                    │
 * │   Moving 상태만 처리                                                   │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ Phase 7. Visualization                                                │
 * │   R: CTransformPrev, CAnimationPrev, CRenderProxyPrev                 │
 * │   → HISM UpdateInstanceTransform + SetCustomDataValue                 │
 * │   ECS → HISM 단방향 동기화 (읽기 전용)                                  │
 * ├─────────────────────────────────────────────────────────────────────────┤
 * │ Phase 8. Cleanup                                                      │
 * │   R: CEnemyStatePrev (Dead 필터), CRenderProxyPrev (InstanceIndex)     │
 * │   W: CRenderProxy.Current (swap 보정, O(1) 룩업 테이블)                │
 * │   내림차순 HISM RemoveInstance → InstanceToEntity 동기화 → Entity 파괴  │
 * └─────────────────────────────────────────────────────────────────────────┘
 */
void FEnemyScheduler::Tick(float DeltaTime)
{
	UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
	if (!World) { return; }

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

	// 2. Phase_Damage (큐 소비 → CHealth 감산 → PushToPrev)
	DamageSystem::Tick(Registry, DamageQueue, InstanceToEntity);

	// 3. Phase_AI
	AISystem::Tick(Registry, DeltaTime, PlayerPosition, AttackRange);

	// 4. Phase_Death (마킹: Dying → Dead)
	DeathSystem::Tick(Registry);

	// 5. Phase_Animation
	AnimationSystem::Tick(Registry, DeltaTime);

	// 6. Phase_Movement
	MovementSystem::Tick(Registry, DeltaTime);

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
