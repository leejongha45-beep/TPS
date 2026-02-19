#include "TPSPlayerLocationProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Enemy/Mass/Fragment/TPSPlayerLocationSharedFragment.h"
#include "Enemy/Mass/Fragment/TPSEnemyHealthFragment.h"
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
	// ① 플레이어 위치 가져오기
	const UWorld* pWorld = EntityManager.GetWorld();
	if (!ensure(pWorld)) return;

	const APawn* pPlayer = UGameplayStatics::GetPlayerPawn(pWorld, 0);
	if (!pPlayer) return;

	const FVector PlayerLocation = pPlayer->GetActorLocation();

	// ② SharedFragment에 저장
	EntityQuery.ForEachEntityChunk(Context,
		[&PlayerLocation](FMassExecutionContext& Context)
		{
			FTPSPlayerLocationSharedFragment& SharedLoc = Context.GetMutableSharedFragment<FTPSPlayerLocationSharedFragment>();
			SharedLoc.PlayerLocation = PlayerLocation;
		});
}