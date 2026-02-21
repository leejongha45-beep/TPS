#include "TPSEnemyDeathProcessor.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "TPSEnemyMeleeProcessor.h"
#include "Enemy/Mass/Fragment/TPSEnemyAIStateFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyHealthFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyLODFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyActorRefFragment.h"
#include "Enemy/Actor/TPSEnemyPawnBase.h"
#include "Enemy/Pool/TPSEnemyActorPoolSubsystem.h"
#include "Enemy/Visualization/TPSEnemyISMSubsystem.h"


// static 델리게이트 정의
FOnEnemyKilled UTPSEnemyDeathProcessor::OnEnemyKilledDelegate;

UTPSEnemyDeathProcessor::UTPSEnemyDeathProcessor()
	: EntityQuery(*this)
{
	// ★ GameThread 필수 — Actor 풀 반환 + Entity 파괴
	bRequiresGameThreadExecution = true;

	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;

	// MeleeProcessor 이후 실행 (공격 판정 완료 후 사망 처리)
	ExecutionOrder.ExecuteAfter.Add(UTPSEnemyMeleeProcessor::StaticClass()->GetFName());
}

void UTPSEnemyDeathProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTPSEnemyAIStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTPSEnemyHealthFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTPSEnemyLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTPSEnemyActorRefFragment>(EMassFragmentAccess::ReadWrite);
}

void UTPSEnemyDeathProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* pWorld = EntityManager.GetWorld();
	if (!ensure(pWorld)) return;

	UTPSEnemyActorPoolSubsystem* pPool = pWorld->GetSubsystem<UTPSEnemyActorPoolSubsystem>();
	if (!ensure(pPool)) return;

	UTPSEnemyISMSubsystem* pISM = pWorld->GetSubsystem<UTPSEnemyISMSubsystem>();
	if (!ensure(pISM)) return;

	TArray<FMassEntityHandle> EntitiesToDestroy;

	EntityQuery.ForEachEntityChunk(Context,
		[pPool, pISM, &EntitiesToDestroy](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();

			const auto AIStateList = Context.GetFragmentView<FTPSEnemyAIStateFragment>();
			const auto LODList = Context.GetFragmentView<FTPSEnemyLODFragment>();
			auto ActorRefList = Context.GetMutableFragmentView<FTPSEnemyActorRefFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FTPSEnemyAIStateFragment& AI = AIStateList[i];

				// Die 상태가 아니면 스킵
				if (AI.AIState != EEnemyAIState::Die) continue;

				const FTPSEnemyLODFragment& LOD = LODList[i];
				FTPSEnemyActorRefFragment& ActorRef = ActorRefList[i];

				// ① FullActor면 Actor 비활성화 + 풀 반환
				if (LOD.LODLevel == EEnemyLODLevel::FullActor && ActorRef.ActorRef.IsValid())
				{
					ATPSEnemyPawnBase* pEnemy = Cast<ATPSEnemyPawnBase>(ActorRef.ActorRef.Get());
					if (ensure(pEnemy))
					{
						pEnemy->DeactivateEnemy();
						pPool->ReturnEnemy(pEnemy);
					}
					ActorRef.ActorRef = nullptr;
				}

				// ② ISM이면 인스턴스 제거
				if (LOD.LODLevel == EEnemyLODLevel::ISM && LOD.ISMInstanceIndex != INDEX_NONE)
				{
					pISM->RemoveInstance(LOD.ISMInstanceIndex);
				}

				// ③ 킬 통보 — 델리게이트 Broadcast (WaveManager 등 외부 시스템이 수신)
				OnEnemyKilledDelegate.Broadcast();

				// ④ Entity 파괴 대상 수집
				EntitiesToDestroy.Add(Context.GetEntity(i));
			}
		});

	// ⑤ 일괄 Entity 파괴
	for (const FMassEntityHandle& Entity : EntitiesToDestroy)
	{
		EntityManager.DestroyEntity(Entity);
	}
}
