#include "TPSPlayerLocationProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Enemy/Mass/Fragment/TPSPlayerLocationSharedFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyHealthFragment.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"
#include "Utils/Interface/Data/Targetable.h"
#include "Kismet/GameplayStatics.h"

UTPSPlayerLocationProcessor::UTPSPlayerLocationProcessor()
	: EntityQuery(*this)
{
	// GameThread 필수 (GetPlayerPawn 호출)
	bRequiresGameThreadExecution = true;

	// 다른 Processor보다 먼저 실행
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UTPSPlayerLocationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// SharedFragment를 쓰기 위해 아무 Fragment 하나 필요 (HealthFragment 사용)
	EntityQuery.AddRequirement<FTPSEnemyHealthFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSharedRequirement<FTPSPlayerLocationSharedFragment>(EMassFragmentAccess::ReadWrite);
}

void UTPSPlayerLocationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// ① 월드 / 플레이어 위치
	const UWorld* pWorld = EntityManager.GetWorld();
	if (!ensure(pWorld)) return;

	const APawn* pPlayer = UGameplayStatics::GetPlayerPawn(pWorld, 0);
	if (!pPlayer) return;

	const FVector PlayerLocation = pPlayer->GetActorLocation();
	const double CurrentTime = pWorld->GetTimeSeconds();

	// ② Subsystem 캐시 (100ms 주기 갱신에 사용)
	const UTPSTargetSubsystem* TargetSS = pWorld->GetSubsystem<UTPSTargetSubsystem>();

	// ③ SharedFragment 갱신
	EntityQuery.ForEachEntityChunk(Context,
		[&PlayerLocation, &CurrentTime, &TargetSS](FMassExecutionContext& Context)
		{
			FTPSPlayerLocationSharedFragment& SharedLoc =
				Context.GetMutableSharedFragment<FTPSPlayerLocationSharedFragment>();

			// ④ 플레이어 위치 + 기지 위치 — 매 프레임 갱신 (가벼움)
			SharedLoc.PlayerLocation = PlayerLocation;
			if (TargetSS)
			{
				SharedLoc.AllyBaseLocation = TargetSS->GetAllyBaseLocation();
			}

			// ⑤ ITargetable 배열 — 100ms 주기로만 갱신
			if (!TargetSS
				|| CurrentTime - SharedLoc.LastTargetUpdateTime
					< FTPSPlayerLocationSharedFragment::TargetUpdateInterval)
			{
				return;
			}
			SharedLoc.LastTargetUpdateTime = CurrentTime;

			// ⑥ Subsystem에서 등록된 타겟만 순회 (TActorIterator 전체 순회 회피)
			int32 Count = 0;
			for (int32 i = TargetSS->GetTargetableActors().Num() - 1; i >= 0; --i)
			{
				AActor* Actor = TargetSS->GetTargetableActors()[i].Get();
				if (!Actor) continue;

				const ITargetable* Target = Cast<ITargetable>(Actor);
				if (Target && Target->IsTargetable()
					&& Count < FTPSPlayerLocationSharedFragment::MaxTargets)
				{
					SharedLoc.TargetLocations[Count++] = Target->GetTargetLocation();
				}
			}
			SharedLoc.TargetCount = Count;
		});
}