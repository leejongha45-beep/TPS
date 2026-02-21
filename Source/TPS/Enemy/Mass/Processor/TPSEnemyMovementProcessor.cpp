#include "TPSEnemyMovementProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "TPSEnemyAIProcessor.h"
#include "Enemy/Mass/Fragment/TPSEnemyAIStateFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyMovementFragment.h"


UTPSEnemyMovementProcessor::UTPSEnemyMovementProcessor()
	: EntityQuery(*this)
{
	// ★ 워커 스레드 — 순수 FVector 연산만
	bRequiresGameThreadExecution = false;

	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;

	// AIProcessor 이후 실행 (TargetLocation 설정 후)
	ExecutionOrder.ExecuteAfter.Add(UTPSEnemyAIProcessor::StaticClass()->GetFName());
}

void UTPSEnemyMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTPSEnemyAIStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTPSEnemyMovementFragment>(EMassFragmentAccess::ReadWrite);
}

void UTPSEnemyMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = Context.GetDeltaTimeSeconds();

	EntityQuery.ForEachEntityChunk(Context,
		[DeltaTime](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();

			const auto AIStateList = Context.GetFragmentView<FTPSEnemyAIStateFragment>();
			auto MovementList = Context.GetMutableFragmentView<FTPSEnemyMovementFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FTPSEnemyAIStateFragment& AI = AIStateList[i];
				FTPSEnemyMovementFragment& Movement = MovementList[i];

				// Chase 상태만 이동
				if (AI.AIState != EEnemyAIState::Chase) continue;

				// TargetLocation 방향으로 이동
				const FVector Direction = (Movement.TargetLocation - Movement.CurrentLocation).GetSafeNormal();
				Movement.CurrentLocation += Direction * Movement.MoveSpeed * DeltaTime;
			}
		});
}