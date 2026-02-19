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
	CurrentCycle = 0;
	TotalWaveNumber = 0;
	AliveEnemies = 0;
	PendingSpawnCount = 0;
	TotalKillCount = 0;

	// 스폰 포인트 비활성화 델리게이트 바인딩
	if (ensure(InWorld))
	{
		UTPSSpawnPointSubsystem* pSpawnSubsystem = InWorld->GetSubsystem<UTPSSpawnPointSubsystem>();
		if (ensure(pSpawnSubsystem))
		{
			pSpawnSubsystem->OnSpawnPointsDeactivatedDelegate.AddUObject(
				this, &UTPSWaveManager::HandleSpawnPointsDeactivated);
		}
	}
}

void UTPSWaveManager::StartWaves()
{
	if (!ensure(WaveConfig)) return;
	if (!ensure(WaveConfig->Waves.Num() > 0)) return;

	CurrentWaveIndex = 0;
	CurrentCycle = 0;
	TotalWaveNumber = 0;
	AliveEnemies = 0;
	PendingSpawnCount = 0;

	StartNextWaveCountdown();
}

void UTPSWaveManager::OnCountdownFinished()
{
	// ① 현재 웨이브 스폰
	SpawnWaveEnemies();

	// ② 다음 웨이브 인덱스 진행 (순환 처리)
	AdvanceWaveIndex();

	// ③ 다음 웨이브 카운트다운 즉시 예약 (전멸 대기 없이 타이머 기반)
	StartNextWaveCountdown();
}

// ──────────────────────────────────────────────
//  스폰 수 계산
// ──────────────────────────────────────────────

int32 UTPSWaveManager::CalcAdjustedSpawnCount(int32 BaseCount) const
{
	if (!ensure(WaveConfig)) return BaseCount;

	// ① Cycle 기반 배율 적용: BaseCount * Multiplier^Cycle
	const float Multiplier = FMath::Pow(WaveConfig->SpawnCountMultiplier, static_cast<float>(CurrentCycle));
	int32 Adjusted = FMath::RoundToInt(static_cast<float>(BaseCount) * Multiplier);

	// ② 웨이브당 상한 적용 (0 = 무제한)
	if (WaveConfig->MaxSpawnCountPerWave > 0)
	{
		Adjusted = FMath::Min(Adjusted, WaveConfig->MaxSpawnCountPerWave);
	}

	return FMath::Max(Adjusted, 1);
}

int32 UTPSWaveManager::GetSpawnCapacity() const
{
	if (!ensure(WaveConfig)) return 0;

	// 동시 존재 제한 없음 (0)
	if (WaveConfig->MaxAliveEnemies <= 0)
	{
		return TNumericLimits<int32>::Max();
	}

	return FMath::Max(0, WaveConfig->MaxAliveEnemies - AliveEnemies);
}

// ──────────────────────────────────────────────
//  Entity 생성 (공통)
// ──────────────────────────────────────────────

int32 UTPSWaveManager::SpawnEntities(int32 Count)
{
	if (Count <= 0) return 0;
	if (!ensure(WaveConfig)) return 0;
	if (!ensure(CachedWorld.IsValid())) return 0;

	// ① 현재 웨이브의 EntityConfig (순환된 인덱스 기준)
	// 지연 스폰 시에도 마지막으로 스폰한 웨이브의 Config를 사용
	const int32 WaveIdx = FMath::Clamp(CurrentWaveIndex > 0 ? CurrentWaveIndex - 1 : WaveConfig->Waves.Num() - 1,
	                                    0, WaveConfig->Waves.Num() - 1);
	const FTPSWaveEntry& Wave = WaveConfig->Waves[WaveIdx];
	if (!ensure(Wave.EnemyEntityConfig)) return 0;

	// ② 플레이어 위치
	APawn* pPlayer = UGameplayStatics::GetPlayerPawn(CachedWorld.Get(), 0);
	if (!ensure(pPlayer)) return 0;

	const FVector PlayerLoc = pPlayer->GetActorLocation();

	// ③ Mass Entity Manager
	UMassEntitySubsystem* pEntitySubsystem = CachedWorld->GetSubsystem<UMassEntitySubsystem>();
	if (!ensure(pEntitySubsystem)) return 0;

	FMassEntityManager& EntityManager = pEntitySubsystem->GetMutableEntityManager();

	// ④ 아키타입
	const FMassEntityTemplate& Template = Wave.EnemyEntityConfig->GetConfig().GetOrCreateEntityTemplate(
		*CachedWorld.Get());
	const FMassArchetypeHandle& Archetype = Template.GetArchetype();

	// ⑤ 스폰 포인트 서브시스템
	UTPSSpawnPointSubsystem* pSpawnSubsystem = CachedWorld->GetSubsystem<UTPSSpawnPointSubsystem>();

	int32 Spawned = 0;

	// ⑥ 스폰 포인트 기반 or 반경 폴백
	if (CachedSpawnPoints.Num() > 0)
	{
		const int32 NumPoints = CachedSpawnPoints.Num();
		const int32 BasePerPoint = Count / NumPoints;
		int32 Remainder = Count % NumPoints;

		for (int32 PointIdx = 0; PointIdx < NumPoints; ++PointIdx)
		{
			const int32 CountForThisPoint = BasePerPoint + (Remainder > 0 ? 1 : 0);
			if (Remainder > 0) --Remainder;

			for (int32 i = 0; i < CountForThisPoint; ++i)
			{
				FVector SpawnLoc = CachedSpawnPoints[PointIdx]->GetRandomLocationInRadius();
				if (ensure(pSpawnSubsystem))
				{
					SpawnLoc = pSpawnSubsystem->SnapToGround(SpawnLoc);
				}

				FMassEntityHandle NewEntity = EntityManager.CreateEntity(Archetype);

				FTPSEnemyMovementFragment* MoveFrag = EntityManager.GetFragmentDataPtr<FTPSEnemyMovementFragment>(NewEntity);
				if (ensure(MoveFrag))
				{
					MoveFrag->CurrentLocation = SpawnLoc;
					MoveFrag->TargetLocation = PlayerLoc;
				}

				++Spawned;
			}
		}
	}
	else
	{
		// ─── 반경 기반 폴백 ───
		for (int32 i = 0; i < Count; ++i)
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

			++Spawned;
		}
	}

	AliveEnemies += Spawned;
	return Spawned;
}

// ──────────────────────────────────────────────
//  웨이브 스폰 (동시 존재 상한 고려)
// ──────────────────────────────────────────────

void UTPSWaveManager::SpawnWaveEnemies()
{
	if (!ensure(WaveConfig)) return;
	if (!ensure(CachedWorld.IsValid())) return;

	const FTPSWaveEntry& Wave = WaveConfig->Waves[CurrentWaveIndex];

	// ① 보정된 스폰 수
	const int32 AdjustedSpawnCount = CalcAdjustedSpawnCount(Wave.SpawnCount);

	// ② 스폰 포인트 캐시 갱신
	UTPSSpawnPointSubsystem* pSpawnSubsystem = CachedWorld->GetSubsystem<UTPSSpawnPointSubsystem>();
	CachedSpawnPoints.Empty();

	if (ensure(pSpawnSubsystem))
	{
		if (Wave.SpawnPointCount > 0)
		{
			CachedSpawnPoints = pSpawnSubsystem->GetSpawnPointsByDistance(Wave.SpawnPointCount);
		}
		else
		{
			CachedSpawnPoints = pSpawnSubsystem->GetSpawnPointsByDistance(TNumericLimits<int32>::Max());
		}
	}

	// ③ 동시 존재 상한 확인
	const int32 Capacity = GetSpawnCapacity();
	const int32 ImmediateSpawn = FMath::Min(AdjustedSpawnCount, Capacity);
	const int32 Deferred = AdjustedSpawnCount - ImmediateSpawn;

	// ④ 즉시 스폰
	const int32 ActuallySpawned = SpawnEntities(ImmediateSpawn);

	// ⑤ 초과분 대기열에 누적
	PendingSpawnCount += Deferred;

	// ⑥ 델리게이트
	OnWaveChangedDelegate.Broadcast(CurrentWaveIndex, CurrentCycle, AdjustedSpawnCount);

	if (Deferred > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] Wave %d (Cycle %d) — spawned %d, deferred %d (alive: %d, pending: %d)"),
		       TotalWaveNumber, CurrentCycle, ActuallySpawned, Deferred, AliveEnemies, PendingSpawnCount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] Wave %d (Cycle %d) — spawned %d (alive: %d)"),
		       TotalWaveNumber, CurrentCycle, ActuallySpawned, AliveEnemies);
	}
}

// ──────────────────────────────────────────────
//  킬 통보 + 지연 스폰
// ──────────────────────────────────────────────

void UTPSWaveManager::NotifyEnemyKilled()
{
	--AliveEnemies;
	++TotalKillCount;

	// 대기열이 있으면 여유만큼 추가 스폰
	if (PendingSpawnCount > 0)
	{
		DrainPendingSpawns();
	}
}

void UTPSWaveManager::DrainPendingSpawns()
{
	if (PendingSpawnCount <= 0) return;

	const int32 Capacity = GetSpawnCapacity();
	if (Capacity <= 0) return;

	const int32 ToSpawn = FMath::Min(PendingSpawnCount, Capacity);
	const int32 Spawned = SpawnEntities(ToSpawn);

	PendingSpawnCount -= Spawned;

	if (Spawned > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[WaveManager] Deferred spawn: %d entities (pending remaining: %d, alive: %d)"),
		       Spawned, PendingSpawnCount, AliveEnemies);
	}
}

// ──────────────────────────────────────────────
//  스폰 포인트 비활성화 핸들러
// ──────────────────────────────────────────────

void UTPSWaveManager::HandleSpawnPointsDeactivated(int32 DeactivatedCount, int32 ActiveCountBefore)
{
	// ① 캐시에서 비활성화된 포인트 제거
	CachedSpawnPoints.RemoveAll([](ATPSEnemySpawnPoint* Point)
	{
		return !ensure(Point) || !Point->IsActive();
	});

	// ② 대기열 비율 감소 — 비활성화된 포인트 비율만큼 차감
	if (PendingSpawnCount > 0 && ActiveCountBefore > 0)
	{
		const float DeactivatedRatio = static_cast<float>(DeactivatedCount) / static_cast<float>(ActiveCountBefore);
		const int32 Reduction = FMath::RoundToInt(static_cast<float>(PendingSpawnCount) * DeactivatedRatio);

		PendingSpawnCount = FMath::Max(0, PendingSpawnCount - Reduction);

		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] SpawnPoint deactivated — ratio %.1f%%, pending reduced by %d (remaining: %d)"),
		       DeactivatedRatio * 100.f, Reduction, PendingSpawnCount);
	}

	// ③ 활성 포인트가 0이면 대기열 전부 초기화
	if (CachedSpawnPoints.Num() == 0)
	{
		if (PendingSpawnCount > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[WaveManager] All spawn points deactivated — clearing pending queue (%d)"),
			       PendingSpawnCount);
		}
		PendingSpawnCount = 0;
	}
}

// ──────────────────────────────────────────────
//  웨이브 진행
// ──────────────────────────────────────────────

void UTPSWaveManager::AdvanceWaveIndex()
{
	if (!ensure(WaveConfig)) return;

	++CurrentWaveIndex;

	// 순환 체크
	if (CurrentWaveIndex >= WaveConfig->Waves.Num())
	{
		OnCycleCompletedDelegate.Broadcast(CurrentCycle);

		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] Cycle %d completed! Starting Cycle %d"),
		       CurrentCycle, CurrentCycle + 1);

		++CurrentCycle;
		CurrentWaveIndex = 0;
	}
}

void UTPSWaveManager::StartNextWaveCountdown()
{
	if (!ensure(WaveConfig)) return;
	if (!ensure(CachedWorld.IsValid())) return;

	++TotalWaveNumber;

	const float Countdown = WaveConfig->Waves[CurrentWaveIndex].CountdownTime;
	CachedWorld->GetTimerManager().SetTimer(
		CountdownTimerHandle, this,
		&UTPSWaveManager::OnCountdownFinished, Countdown, false);
}
