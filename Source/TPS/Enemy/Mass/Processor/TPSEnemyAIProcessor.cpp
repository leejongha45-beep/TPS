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

				if (AI.AIState == EEnemyAIState::Die) continue;

				const FVector& EntityLocation = Movement.CurrentLocation;
				const int32 TargetCount = SharedLoc.TargetCount;

				// ──────────── 타겟 결정 ────────────

				// ① 피격 어그로 최우선
				if (AI.bHasHitAggro)
				{
					AI.CurrentTargetLocation = AI.HitAggroLocation;
					AI.CurrentTargetIndex = INDEX_NONE;

					const float DistToHit = FVector::Dist(EntityLocation, AI.HitAggroLocation);
					if (DistToHit <= AI.AttackRange || DistToHit > AI.AggroReleaseRange)
					{
						AI.bHasHitAggro = false;
					}
				}
				else
				{
					// ② 이전 타겟 위치 매칭 (배열 변동 방어)
					int32 MatchedIndex = INDEX_NONE;

					if (AI.CurrentTargetIndex != INDEX_NONE)
					{
						constexpr float MatchThresholdSq = 100.f * 100.f;
						for (int32 t = 0; t < TargetCount; ++t)
						{
							if (FVector::DistSquared(AI.CurrentTargetLocation, SharedLoc.TargetLocations[t])
								< MatchThresholdSq)
							{
								MatchedIndex = t;
								break;
							}
						}

						// 매칭 성공 → 히스테리시스 체크
						if (MatchedIndex != INDEX_NONE)
						{
							AI.CurrentTargetIndex = MatchedIndex;
							AI.CurrentTargetLocation = SharedLoc.TargetLocations[MatchedIndex];

							if (FVector::Dist(EntityLocation, AI.CurrentTargetLocation) > AI.AggroReleaseRange)
							{
								MatchedIndex = INDEX_NONE;
								AI.CurrentTargetIndex = INDEX_NONE;
							}
						}
					}

					// ③ 매칭 실패 → 최근접 재탐색
					if (MatchedIndex == INDEX_NONE)
					{
						float BestDistSq = AI.AggroRange * AI.AggroRange;
						int32 BestIndex = INDEX_NONE;

						for (int32 t = 0; t < TargetCount; ++t)
						{
							const float DistSq = FVector::DistSquared(EntityLocation, SharedLoc.TargetLocations[t]);
							if (DistSq < BestDistSq)
							{
								BestDistSq = DistSq;
								BestIndex = t;
							}
						}

						if (BestIndex != INDEX_NONE)
						{
							AI.CurrentTargetIndex = BestIndex;
							AI.CurrentTargetLocation = SharedLoc.TargetLocations[BestIndex];
						}
						else
						{
							// AggroRange 내 타겟 없음 → 기지 진군
							AI.CurrentTargetIndex = INDEX_NONE;
							AI.CurrentTargetLocation = SharedLoc.AllyBaseLocation;
						}
					}
				}

				// ──────────── 이동 목표 + LOD 갱신 ────────────
				Movement.TargetLocation = AI.CurrentTargetLocation;
				const float Distance = FVector::Dist(EntityLocation, AI.CurrentTargetLocation);
				LOD.DistanceToPlayer = FVector::Dist(EntityLocation, PlayerLocation);

				// ──────────── 상태 전환 ────────────
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
