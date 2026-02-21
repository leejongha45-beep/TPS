#include "TPSEnemyLODProcessor.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "TPSEnemyMovementProcessor.h"
#include "Enemy/Mass/Fragment/TPSEnemyLODFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyMovementFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyActorRefFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyAIStateFragment.h"
#include "Enemy/Actor/TPSEnemyPawnBase.h"
#include "Enemy/Pool/TPSEnemyActorPoolSubsystem.h"
#include "Enemy/Visualization/TPSEnemyISMSubsystem.h"


UTPSEnemyLODProcessor::UTPSEnemyLODProcessor()
	: EntityQuery(*this)
{
	// ★ GameThread 필수 — Actor 풀 Get/Return + ISM 호출
	bRequiresGameThreadExecution = true;

	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;

	// MovementProcessor 이후 실행 (위치 갱신 후 LOD 판정)
	ExecutionOrder.ExecuteAfter.Add(UTPSEnemyMovementProcessor::StaticClass()->GetFName());
}

void UTPSEnemyLODProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTPSEnemyLODFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTPSEnemyMovementFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTPSEnemyActorRefFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTPSEnemyAIStateFragment>(EMassFragmentAccess::ReadOnly);
}

void UTPSEnemyLODProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 서브시스템 참조
	UWorld* pWorld = EntityManager.GetWorld();
	if (!ensure(pWorld)) return;

	UTPSEnemyActorPoolSubsystem* pPool = pWorld->GetSubsystem<UTPSEnemyActorPoolSubsystem>();
	if (!ensure(pPool)) return;

	UTPSEnemyISMSubsystem* pISM = pWorld->GetSubsystem<UTPSEnemyISMSubsystem>();
	if (!ensure(pISM)) return;

	int32 TransitionCount = 0;

	EntityQuery.ForEachEntityChunk(Context,
		[pPool, pISM, &TransitionCount, &EntityManager](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();

			const auto AIStateList = Context.GetFragmentView<FTPSEnemyAIStateFragment>();
			const auto MovementList = Context.GetFragmentView<FTPSEnemyMovementFragment>();
			auto LODList = Context.GetMutableFragmentView<FTPSEnemyLODFragment>();
			auto ActorRefList = Context.GetMutableFragmentView<FTPSEnemyActorRefFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FTPSEnemyAIStateFragment& AI = AIStateList[i];
				const FTPSEnemyMovementFragment& Movement = MovementList[i];
				FTPSEnemyLODFragment& LOD = LODList[i];
				FTPSEnemyActorRefFragment& ActorRef = ActorRefList[i];

				// Die 상태면 스킵 (DeathProcessor가 처리)
				if (AI.AIState == EEnemyAIState::Die) continue;

				const float Dist = LOD.DistanceToPlayer;
				const EEnemyLODLevel PrevLOD = LOD.LODLevel;
				EEnemyLODLevel NewLOD = PrevLOD;

				// ① 히스테리시스 적용 LOD 판정
				if (PrevLOD == EEnemyLODLevel::None)
				{
					if (Dist < ISMDistance - Hysteresis) NewLOD = EEnemyLODLevel::ISM;
				}
				else if (PrevLOD == EEnemyLODLevel::ISM)
				{
					if (Dist > ISMDistance + Hysteresis) NewLOD = EEnemyLODLevel::None;
					else if (Dist < FullActorDistance - Hysteresis) NewLOD = EEnemyLODLevel::FullActor;
				}
				else // FullActor
				{
					if (Dist > FullActorDistance + Hysteresis) NewLOD = EEnemyLODLevel::ISM;
				}

				// ② LOD 변경 없으면 스킵
				if (NewLOD == PrevLOD) continue;

				// ③ 프레임당 전환 상한 체크 — 초과 시 다음 프레임으로 미룸
				if (TransitionCount >= MaxTransitionsPerFrame) continue;

				// ④ FullActor 진입 → 풀에서 Actor 꺼내기
				if (NewLOD == EEnemyLODLevel::FullActor && PrevLOD != EEnemyLODLevel::FullActor)
				{
					ATPSEnemyPawnBase* pEnemy = pPool->GetEnemy();
					if (!ensure(pEnemy))
					{
						// Pool 확장 상한 초과 — FullActor 전환 취소, ISM 유지
						NewLOD = EEnemyLODLevel::ISM;
					}
					else
					{
						const FTransform SpawnTransform(FRotator::ZeroRotator, Movement.CurrentLocation);
						pEnemy->ActivateEnemy(SpawnTransform, AI.AIState);
						pEnemy->SetMassEntityHandle(Context.GetEntity(i), &EntityManager);
						ActorRef.ActorRef = pEnemy;
					}
				}
				// ⑤ FullActor 이탈 → Actor 풀에 반환
				else if (PrevLOD == EEnemyLODLevel::FullActor && NewLOD != EEnemyLODLevel::FullActor)
				{
					if (ActorRef.ActorRef.IsValid())
					{
						ATPSEnemyPawnBase* pEnemy = Cast<ATPSEnemyPawnBase>(ActorRef.ActorRef.Get());
						if (ensure(pEnemy))
						{
							pEnemy->DeactivateEnemy();
							pPool->ReturnEnemy(pEnemy);
						}
						ActorRef.ActorRef = nullptr;
					}
				}

				// ⑥ ISM 전환
				if (NewLOD == EEnemyLODLevel::ISM && PrevLOD != EEnemyLODLevel::ISM)
				{
					// ISM 진입 → 인스턴스 추가
					const FTransform ISMTransform(FRotator::ZeroRotator, Movement.CurrentLocation);
					LOD.ISMInstanceIndex = pISM->AddInstance(ISMTransform);
				}
				else if (PrevLOD == EEnemyLODLevel::ISM && NewLOD != EEnemyLODLevel::ISM)
				{
					// ISM 이탈 → 인스턴스 제거
					pISM->RemoveInstance(LOD.ISMInstanceIndex);
					LOD.ISMInstanceIndex = INDEX_NONE;
				}

				LOD.LODLevel = NewLOD;
				++TransitionCount;
			}
		});
}