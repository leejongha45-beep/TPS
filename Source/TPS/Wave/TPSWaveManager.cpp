#include "Wave/TPSWaveManager.h"
#include "Wave/TPSWaveConfig.h"
#include "Wave/TPSEnemySpawnPoint.h"
#include "Wave/TPSSpawnPointSubsystem.h"
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

	// ① 플레이어 위치 (폴백용 + TargetLocation용)
	APawn* pPlayer = UGameplayStatics::GetPlayerPawn(CachedWorld.Get(), 0);
	if (!ensure(pPlayer)) return;

	const FVector PlayerLoc = pPlayer->GetActorLocation();

	// ② Mass Entity Manager 획득
	UMassEntitySubsystem* pEntitySubsystem = CachedWorld->GetSubsystem<UMassEntitySubsystem>();
	if (!ensure(pEntitySubsystem)) return;

	FMassEntityManager& EntityManager = pEntitySubsystem->GetMutableEntityManager();

	// ③ EntityConfig → 아키타입 확보
	const FMassEntityTemplate& Template = Wave.EnemyEntityConfig->GetConfig().GetOrCreateEntityTemplate(
		*CachedWorld.Get());
	const FMassArchetypeHandle& Archetype = Template.GetArchetype();

	RemainingEnemies = Wave.SpawnCount;

	// ④ 스폰 포인트 선택
	UTPSSpawnPointSubsystem* pSpawnSubsystem = CachedWorld->GetSubsystem<UTPSSpawnPointSubsystem>();
	TArray<ATPSEnemySpawnPoint*> SelectedPoints;

	if (ensure(pSpawnSubsystem))
	{
		if (Wave.SpawnPointCount > 0)
		{
			// 가까운 순 N개
			SelectedPoints = pSpawnSubsystem->GetSpawnPointsByDistance(Wave.SpawnPointCount);
		}
		else
		{
			// 전체 활성 포인트
			SelectedPoints = pSpawnSubsystem->GetSpawnPointsByDistance(TNumericLimits<int32>::Max());
		}
	}

	// ⑤ 스폰 포인트 기반 스폰 or 반경 기반 폴백
	if (SelectedPoints.Num() > 0)
	{
		// ─── 포인트 기반 스폰 ───
		const int32 NumPoints = SelectedPoints.Num();
		const int32 BaseCount = Wave.SpawnCount / NumPoints;
		int32 Remainder = Wave.SpawnCount % NumPoints;

		for (int32 PointIdx = 0; PointIdx < NumPoints; ++PointIdx)
		{
			const int32 CountForThisPoint = BaseCount + (Remainder > 0 ? 1 : 0);
			if (Remainder > 0) --Remainder;

			for (int32 i = 0; i < CountForThisPoint; ++i)
			{
				FVector SpawnLoc = SelectedPoints[PointIdx]->GetRandomLocationInRadius();
				SpawnLoc = pSpawnSubsystem->SnapToGround(SpawnLoc);

				FMassEntityHandle NewEntity = EntityManager.CreateEntity(Archetype);

				FTPSEnemyMovementFragment* MoveFrag = EntityManager.GetFragmentDataPtr<FTPSEnemyMovementFragment>(NewEntity);
				if (ensure(MoveFrag))
				{
					MoveFrag->CurrentLocation = SpawnLoc;
					MoveFrag->TargetLocation = PlayerLoc;
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] Wave %d started — %d enemies across %d spawn points"),
		       CurrentWaveIndex + 1, Wave.SpawnCount, NumPoints);
	}
	else
	{
		// ─── 반경 기반 폴백 (기존 로직) ───
		for (int32 i = 0; i < Wave.SpawnCount; ++i)
		{
			const float Angle = FMath::RandRange(0.f, 2.f * PI);
			const float Radius = FMath::RandRange(Wave.SpawnRadius * 0.5f, Wave.SpawnRadius);
			const FVector SpawnLoc = PlayerLoc + FVector(
				FMath::Cos(Angle) * Radius,
				FMath::Sin(Angle) * Radius,
				0.f
			);

			FMassEntityHandle NewEntity = EntityManager.CreateEntity(Archetype);

			FTPSEnemyMovementFragment* MoveFrag = EntityManager.GetFragmentDataPtr<FTPSEnemyMovementFragment>(NewEntity);
			if (ensure(MoveFrag))
			{
				MoveFrag->CurrentLocation = SpawnLoc;
				MoveFrag->TargetLocation = PlayerLoc;
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] Wave %d started — %d enemies (radius fallback, no spawn points)"),
		       CurrentWaveIndex + 1, Wave.SpawnCount);
	}
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