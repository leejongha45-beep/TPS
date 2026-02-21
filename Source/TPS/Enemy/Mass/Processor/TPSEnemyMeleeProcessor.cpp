#include "TPSEnemyMeleeProcessor.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "TPSEnemyTransformSyncProcessor.h"
#include "Enemy/Mass/Fragment/TPSEnemyAIStateFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyLODFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyActorRefFragment.h"
#include "Core/Subsystem/TPSDamageSubsystem.h"


UTPSEnemyMeleeProcessor::UTPSEnemyMeleeProcessor()
	: EntityQuery(*this)
{
	// ★ GameThread 필수 — ReceiveDamage 호출
	bRequiresGameThreadExecution = true;

	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;

	// TransformSyncProcessor 이후 실행 (Actor 위치 동기화 완료 후)
	ExecutionOrder.ExecuteAfter.Add(UTPSEnemyTransformSyncProcessor::StaticClass()->GetFName());
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

	const UTPSDamageSubsystem* DamageSS = pWorld->GetSubsystem<UTPSDamageSubsystem>();
	if (!ensure(DamageSS)) return;

	const float DeltaTime = Context.GetDeltaTimeSeconds();

	EntityQuery.ForEachEntityChunk(Context,
		[DamageSS, DeltaTime](FMassExecutionContext& Context)
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

				// ① CurrentTargetLocation과 가장 가까운 IDamageable 탐색
				IDamageable* BestTarget = nullptr;
				float BestDistSq = AI.AttackRange * AI.AttackRange;

				for (const TScriptInterface<IDamageable>& Damageable : DamageSS->GetDamageableActors())
				{
					if (!Damageable || !Damageable->IsDamageable()) continue;

					const float DistSq = FVector::DistSquared(
						AI.CurrentTargetLocation, Damageable->GetDamageableLocation());
					if (DistSq < BestDistSq)
					{
						BestDistSq = DistSq;
						BestTarget = Damageable.GetInterface();
					}
				}

				if (!BestTarget) continue;

				// ② 공격 실행 — IDamageable 인터페이스 직접 호출
				AI.AttackCooldownTimer = AI.AttackInterval;
				BestTarget->ReceiveDamage(AI.AttackDamage, ActorRef.ActorRef.Get());
			}
		});
}