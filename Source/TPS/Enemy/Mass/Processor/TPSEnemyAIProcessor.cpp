#include "TPSEnemyAIProcessor.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "TPSPlayerLocationProcessor.h"
#include "Enemy/Mass/Fragment/TPSEnemyAIStateFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyMovementFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyLODFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyHealthFragment.h"
#include "Enemy/Mass/Fragment/TPSPlayerLocationSharedFragment.h"

UTPSEnemyAIProcessor::UTPSEnemyAIProcessor()
	: EntityQuery(*this)
{
	// ★ 워커 스레드 실행 — 순수 float 연산만
	bRequiresGameThreadExecution = false;

	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;

	// PlayerLocationProcessor 이후 실행
	ExecutionOrder.ExecuteAfter.Add(UTPSPlayerLocationProcessor::StaticClass()->GetFName());
}

void UTPSEnemyAIProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTPSEnemyAIStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTPSEnemyMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTPSEnemyLODFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTPSEnemyHealthFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSharedRequirement<FTPSPlayerLocationSharedFragment>(EMassFragmentAccess::ReadOnly);
}

void UTPSEnemyAIProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = Context.GetDeltaTimeSeconds();

	EntityQuery.ForEachEntityChunk(Context,
		[DeltaTime](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();

			// SharedFragment에서 플레이어 위치 읽기
			const FTPSPlayerLocationSharedFragment& SharedLoc = Context.GetSharedFragment<FTPSPlayerLocationSharedFragment>();
			const FVector& PlayerLocation = SharedLoc.PlayerLocation;

			// Fragment 배열 참조
			const auto HealthList = Context.GetFragmentView<FTPSEnemyHealthFragment>();
			auto AIStateList = Context.GetMutableFragmentView<FTPSEnemyAIStateFragment>();
			auto MovementList = Context.GetMutableFragmentView<FTPSEnemyMovementFragment>();
			auto LODList = Context.GetMutableFragmentView<FTPSEnemyLODFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				FTPSEnemyAIStateFragment& AI = AIStateList[i];
				FTPSEnemyMovementFragment& Movement = MovementList[i];
				FTPSEnemyLODFragment& LOD = LODList[i];

				// Die 상태면 스킵
				if (AI.AIState == EEnemyAIState::Die) continue;

				// ① 거리 계산 + 캐싱
				const float Distance = FVector::Dist(Movement.CurrentLocation, PlayerLocation);
				LOD.DistanceToPlayer = Distance;
				Movement.TargetLocation = PlayerLocation;

				// ② 상태 전환
				switch (AI.AIState)
				{
				case EEnemyAIState::Idle:
					AI.StateTimer += DeltaTime;
					if (AI.StateTimer >= AI.IdleWaitTime)
					{
						AI.AIState = EEnemyAIState::Chase;
					}
					break;

				case EEnemyAIState::Chase:
					if (Distance <= AI.AttackRange)
					{
						AI.AIState = EEnemyAIState::Attack;
					}
					break;

				case EEnemyAIState::Attack:
					if (Distance > AI.AttackRange * AI.AttackRangeHysteresis)
					{
						AI.AIState = EEnemyAIState::Chase;
					}
					break;

				default:
					break;
				}
			}
		});
}
