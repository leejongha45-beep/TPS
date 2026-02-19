#include "TPSEnemyTransformSyncProcessor.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "TPSEnemyLODProcessor.h"
#include "Enemy/Mass/Fragment/TPSEnemyMovementFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyLODFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyActorRefFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyAIStateFragment.h"
#include "Enemy/Mass/Fragment/TPSPlayerLocationSharedFragment.h"
#include "Enemy/Actor/TPSEnemyPawnBase.h"
#include "Enemy/Visualization/TPSEnemyISMSubsystem.h"

UTPSEnemyTransformSyncProcessor::UTPSEnemyTransformSyncProcessor()
	: EntityQuery(*this)
{
	// ★ GameThread 필수 — Actor SetActorLocation + ISM UpdateTransform
	bRequiresGameThreadExecution = true;

	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;

	// LODProcessor 이후 실행 (LOD 전환 확정 후 위치 반영)
	ExecutionOrder.ExecuteAfter.Add(UTPSEnemyLODProcessor::StaticClass()->GetFName());
}

void UTPSEnemyTransformSyncProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTPSEnemyMovementFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTPSEnemyLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTPSEnemyActorRefFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTPSEnemyAIStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSharedRequirement<FTPSPlayerLocationSharedFragment>(EMassFragmentAccess::ReadOnly);
}

void UTPSEnemyTransformSyncProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* pWorld = EntityManager.GetWorld();
	if (!ensure(pWorld)) return;

	UTPSEnemyISMSubsystem* pISM = pWorld->GetSubsystem<UTPSEnemyISMSubsystem>();
	if (!ensure(pISM)) return;

	EntityQuery.ForEachEntityChunk(Context,
		[pISM](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();

			const auto AIStateList = Context.GetFragmentView<FTPSEnemyAIStateFragment>();
			const auto MovementList = Context.GetFragmentView<FTPSEnemyMovementFragment>();
			const auto LODList = Context.GetFragmentView<FTPSEnemyLODFragment>();
			const auto ActorRefList = Context.GetFragmentView<FTPSEnemyActorRefFragment>();
			const auto& SharedLocation = Context.GetSharedFragment<FTPSPlayerLocationSharedFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FTPSEnemyAIStateFragment& AI = AIStateList[i];

				// Die 상태면 스킵
				if (AI.AIState == EEnemyAIState::Die) continue;

				const FTPSEnemyMovementFragment& Movement = MovementList[i];
				const FTPSEnemyLODFragment& LOD = LODList[i];
				const FTPSEnemyActorRefFragment& ActorRef = ActorRefList[i];

				// ① FullActor → Actor 위치/회전/AI상태 동기화
				if (LOD.LODLevel == EEnemyLODLevel::FullActor && ActorRef.ActorRef.IsValid())
				{
					ATPSEnemyPawnBase* pEnemy = Cast<ATPSEnemyPawnBase>(ActorRef.ActorRef.Get());
					if (!ensure(pEnemy)) continue;

					pEnemy->SetActorLocation(Movement.CurrentLocation);

					// 플레이어 방향으로 회전
					const FVector Direction = (SharedLocation.PlayerLocation - Movement.CurrentLocation).GetSafeNormal2D();
					if (!Direction.IsNearlyZero())
					{
						pEnemy->SetActorRotation(Direction.Rotation());
					}

					// AI 상태 동기화 (시각/사운드 전환)
					pEnemy->SyncAIState(AI.AIState);
				}
				// ② ISM → ISM Transform 갱신
				else if (LOD.LODLevel == EEnemyLODLevel::ISM && LOD.ISMInstanceIndex != INDEX_NONE)
				{
					const FTransform ISMTransform(FRotator::ZeroRotator, Movement.CurrentLocation);
					pISM->UpdateInstanceTransform(LOD.ISMInstanceIndex, ISMTransform);
				}
			}
		});
}
