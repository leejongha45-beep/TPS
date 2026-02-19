#include "TPSEnemyMeleeProcessor.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "TPSEnemyLODProcessor.h"
#include "Enemy/Mass/Fragment/TPSEnemyAIStateFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyLODFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyActorRefFragment.h"
#include "Kismet/GameplayStatics.h"

UTPSEnemyMeleeProcessor::UTPSEnemyMeleeProcessor()
	: EntityQuery(*this)
{
	// ★ GameThread 필수 — ApplyDamage 호출
	bRequiresGameThreadExecution = true;

	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;

	// LODProcessor 이후 실행 (Actor 스폰 확정 후)
	ExecutionOrder.ExecuteAfter.Add(UTPSEnemyLODProcessor::StaticClass()->GetFName());
}

void UTPSEnemyMeleeProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTPSEnemyAIStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTPSEnemyLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTPSEnemyActorRefFragment>(EMassFragmentAccess::ReadOnly);
}

void UTPSEnemyMeleeProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* pWorld = EntityManager.GetWorld();
	if (!ensure(pWorld)) return;

	// TODO: 나중에 NPC 타겟도 지원할 때 타겟 선택 로직 확장
	APawn* pTarget = UGameplayStatics::GetPlayerPawn(pWorld, 0);
	if (!pTarget) return;

	const float DeltaTime = Context.GetDeltaTimeSeconds();

	EntityQuery.ForEachEntityChunk(Context,
		[pTarget, DeltaTime](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();

			auto AIStateList = Context.GetMutableFragmentView<FTPSEnemyAIStateFragment>();
			const auto LODList = Context.GetFragmentView<FTPSEnemyLODFragment>();
			const auto ActorRefList = Context.GetFragmentView<FTPSEnemyActorRefFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				FTPSEnemyAIStateFragment& AI = AIStateList[i];
				const FTPSEnemyLODFragment& LOD = LODList[i];
				const FTPSEnemyActorRefFragment& ActorRef = ActorRefList[i];

				// Attack 상태가 아니면 스킵
				if (AI.AIState != EEnemyAIState::Attack) continue;

				// FullActor가 아니면 스킵 (안전장치 — 공격 사거리 < FullActor 거리)
				if (LOD.LODLevel != EEnemyLODLevel::FullActor) continue;

				// Actor 참조 유효성 체크
				if (!ActorRef.ActorRef.IsValid()) continue;

				// 쿨다운 감산
				AI.AttackCooldownTimer -= DeltaTime;
				if (AI.AttackCooldownTimer > 0.f) continue;

				// 공격 실행
				AI.AttackCooldownTimer = AI.AttackInterval;
				UGameplayStatics::ApplyDamage(
					pTarget,
					AI.AttackDamage,
					nullptr,
					ActorRef.ActorRef.Get(),
					nullptr
				);
			}
		});
}
