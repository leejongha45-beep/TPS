#include "Wave/TPSWaveManager.h"
#include "Wave/TPSWaveConfig.h"
#include "MassEntitySubsystem.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityTemplate.h"
#include "Enemy/Mass/Fragment/TPSEnemyMovementFragment.h"
#include "Kismet/GameplayStatics.h"

void UTPSWaveManager::Initialize(UWorld* InWorld, UTPSWaveConfig* InConfig)
{
	CachedWorld = InWorld;
	WaveConfig = InConfig;
	CurrentWaveIndex = 0;
	TotalKillCount = 0;
}

void UTPSWaveManager::StartWaves()
{
	if (!ensure(WaveConfig)) return;
	if (!ensure(WaveConfig->Waves.Num() > 0)) return;

	CurrentWaveIndex = 0;
	OnWaveChangedDelegate.Broadcast(CurrentWaveIndex, WaveConfig->Waves.Num());

	// 카운트다운 시작
	const float Countdown = WaveConfig->Waves[CurrentWaveIndex].CountdownTime;
	if (ensure(CachedWorld.IsValid()))
	{
		CachedWorld->GetTimerManager().SetTimer(
			CountdownTimerHandle, this,
			&UTPSWaveManager::OnCountdownFinished, Countdown, false);
	}
}

void UTPSWaveManager::OnCountdownFinished()
{
	SpawnWaveEnemies();
}

void UTPSWaveManager::SpawnWaveEnemies()
{
	if (!ensure(WaveConfig)) return;
	if (!ensure(CachedWorld.IsValid())) return;

	const FTPSWaveEntry& Wave = WaveConfig->Waves[CurrentWaveIndex];
	if (!ensure(Wave.EnemyEntityConfig)) return;

	// 플레이어 위치 기준 스폰
	APawn* pPlayer = UGameplayStatics::GetPlayerPawn(CachedWorld.Get(), 0);
	if (!ensure(pPlayer)) return;

	const FVector PlayerLoc = pPlayer->GetActorLocation();

	// Mass Entity Manager 획득
	UMassEntitySubsystem* pEntitySubsystem = CachedWorld->GetSubsystem<UMassEntitySubsystem>();
	if (!ensure(pEntitySubsystem)) return;

	FMassEntityManager& EntityManager = pEntitySubsystem->GetMutableEntityManager();

	// EntityConfig → 아키타입 확보
	const FMassEntityTemplate& Template = Wave.EnemyEntityConfig->GetConfig().GetOrCreateEntityTemplate(
		*CachedWorld.Get());
	const FMassArchetypeHandle& Archetype = Template.GetArchetype();

	RemainingEnemies = Wave.SpawnCount;

	for (int32 i = 0; i < Wave.SpawnCount; ++i)
	{
		// 랜덤 원형 분포
		const float Angle = FMath::RandRange(0.f, 2.f * PI);
		const float Radius = FMath::RandRange(Wave.SpawnRadius * 0.5f, Wave.SpawnRadius);
		const FVector SpawnLoc = PlayerLoc + FVector(
			FMath::Cos(Angle) * Radius,
			FMath::Sin(Angle) * Radius,
			0.f
			);

		// Entity 생성
		FMassEntityHandle NewEntity = EntityManager.CreateEntity(Archetype);

		// Movement Fragment 초기 위치 설정
		FTPSEnemyMovementFragment* MoveFrag = EntityManager.GetFragmentDataPtr<FTPSEnemyMovementFragment>(NewEntity);
		if (ensure(MoveFrag))
		{
			MoveFrag->CurrentLocation = SpawnLoc;
			MoveFrag->TargetLocation = PlayerLoc;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[WaveManager] Wave %d started — %d enemies"),
	       CurrentWaveIndex + 1, Wave.SpawnCount);
}

void UTPSWaveManager::NotifyEnemyKilled()
{
	--RemainingEnemies;
	++TotalKillCount;

	CheckWaveClear();
}

void UTPSWaveManager::CheckWaveClear()
{
	if (RemainingEnemies > 0) return;

	UE_LOG(LogTemp, Warning, TEXT("[WaveManager] Wave %d cleared! Total kills: %d"),
	       CurrentWaveIndex + 1, TotalKillCount);

	++CurrentWaveIndex;

	// 올 클리어 체크
	if (!ensure(WaveConfig)) return;
	if (CurrentWaveIndex >= WaveConfig->Waves.Num())
	{
		OnAllWavesClearedDelegate.Broadcast();
		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] All waves cleared!"));
		return;
	}

	// 다음 웨이브
	OnWaveChangedDelegate.Broadcast(CurrentWaveIndex, WaveConfig->Waves.Num());

	const float Countdown = WaveConfig->Waves[CurrentWaveIndex].CountdownTime;
	if (ensure(CachedWorld.IsValid()))
	{
		CachedWorld->GetTimerManager().SetTimer(
			CountdownTimerHandle, this,
			&UTPSWaveManager::OnCountdownFinished, Countdown, false);
	}
}