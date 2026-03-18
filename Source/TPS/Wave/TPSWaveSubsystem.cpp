// TPSWaveSubsystem.cpp

#include "Wave/TPSWaveSubsystem.h"
#include "Wave/TPSWaveSettings.h"
#include "Engine/World.h"
#include "ECS/Data/TPSEnemyTypeDataAsset.h"
#include "ECS/Scheduler/EnemyManagerSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Wave/TPSEnemySpawnPoint.h"
#include "EngineUtils.h"

void UTPSWaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ① WaveSettings CDO 캐시
	CachedSettingsInst = GetDefault<UTPSWaveSettings>();

	// ② DataAsset 동기 로드
	if (ensure(CachedSettingsInst.Get()))
	{
		for (const TSoftObjectPtr<UTPSEnemyTypeDataAsset>& SoftRef : CachedSettingsInst->EnemyTypes)
		{
			UTPSEnemyTypeDataAsset* pLoaded = SoftRef.LoadSynchronous();
			if (ensure(pLoaded))
			{
				LoadedEnemyTypes.Add(pLoaded);
			}
		}
	}
}

void UTPSWaveSubsystem::Deinitialize()
{
	StopWaveSystem();
	LoadedEnemyTypes.Empty();
	Super::Deinitialize();
}

TStatId UTPSWaveSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTPSWaveSubsystem, STATGROUP_Tickables);
}

void UTPSWaveSubsystem::StartWaveSystem()
{
	if (bIsActive) { return; }

	// ① 스폰 포인트 수집
	CollectSpawnPoints();

	UE_LOG(LogTemp, Warning, TEXT("[Wave] StartWaveSystem — SpawnPoints=%d, EnemyTypes=%d"),
		CachedSpawnPoints.Num(), LoadedEnemyTypes.Num());

	// ② 상태 초기화
	bIsActive = true;
	CurrentPhase = EWavePhase::Trickle;
	CurrentWaveLevel = 0;
	ElapsedTime = 0.f;
	LastTrickleSpawnTime = 0.f;
	LastBigWaveTime = 0.f;
	BigWaveAlertEndTime = 0.f;
	BigWaveGroupsRemaining = 0;
}

void UTPSWaveSubsystem::StopWaveSystem()
{
	bIsActive = false;
	CurrentPhase = EWavePhase::Idle;
	PendingSpawnQueue.Empty();
	BigWaveGroupsRemaining = 0;
}

void UTPSWaveSubsystem::Tick(float DeltaTime)
{
	if (!bIsActive) { return; }

	ElapsedTime += DeltaTime;

	if (!ensure(CachedSettingsInst.Get())) { return; }

	// ① 빅웨이브 주기 체크 — Alert 전환
	const float TimeSinceLastBigWave = ElapsedTime - LastBigWaveTime;
	if (TimeSinceLastBigWave >= CachedSettingsInst->BigWavePeriod
	    && CurrentPhase != EWavePhase::BigWaveAlert
	    && CurrentPhase != EWavePhase::BigWaveActive)
	{
		++CurrentWaveLevel;
		CurrentPhase = EWavePhase::BigWaveAlert;
		BigWaveAlertEndTime = ElapsedTime + CachedSettingsInst->BigWaveAlertDuration;
		LastBigWaveTime = ElapsedTime;
	}

	// ② Alert → Active 전환 (경고 시간 경과 후)
	if (CurrentPhase == EWavePhase::BigWaveAlert && ElapsedTime >= BigWaveAlertEndTime)
	{
		CurrentPhase = EWavePhase::BigWaveActive;

		const int32 TotalCount = CachedSettingsInst->BigWaveBaseCount
		    + CurrentWaveLevel * CachedSettingsInst->BigWaveCountPerLevel;
		const int32 GroupCount = FMath::Max(CachedSettingsInst->BigWaveGroupCount, 1);
		const int32 PerGroup = TotalCount / GroupCount;

		BigWaveGroupsRemaining = GroupCount;
		NextBigWaveGroupTime = ElapsedTime;

		// 첫 그룹 즉시 예약
		ScheduleSpawnGroup(PerGroup, ElapsedTime, CachedSettingsInst->TrickleGroupSpawnDelay);
		--BigWaveGroupsRemaining;
		NextBigWaveGroupTime += CachedSettingsInst->BigWaveGroupInterval;
	}

	// ③ 빅웨이브 그룹 스폰 진행
	TickBigWave();

	// ④ 트리클 스폰 (항상 동작)
	TickTrickleSpawn();

	// ⑤ 예약된 스폰 실행
	FlushReadySpawns();
}

void UTPSWaveSubsystem::TickTrickleSpawn()
{
	if (ElapsedTime - LastTrickleSpawnTime < CachedSettingsInst->TrickleSpawnInterval) { return; }

	// MaxEnemyCount 캡 체크
	UEnemyManagerSubsystem* pEnemySub = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
	if (!ensure(pEnemySub))
	{
		UE_LOG(LogTemp, Error, TEXT("[Wave] TickTrickle — EnemyManagerSubsystem NULL"));
		return;
	}

	UInstancedStaticMeshComponent* pHISM = pEnemySub->GetHISM();
	if (!ensure(pHISM))
	{
		UE_LOG(LogTemp, Error, TEXT("[Wave] TickTrickle — HISM NULL"));
		return;
	}

	const int32 CurrentCount = pHISM->GetInstanceCount() + PendingSpawnQueue.Num();
	if (CurrentCount >= CachedSettingsInst->MaxEnemyCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Wave] TickTrickle — MaxEnemyCount cap hit (%d/%d)"),
			CurrentCount, CachedSettingsInst->MaxEnemyCount);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Wave] TickTrickle — Scheduling group: Size=%d, Elapsed=%.2f"),
		CachedSettingsInst->TrickleGroupSize, ElapsedTime);

	ScheduleSpawnGroup(CachedSettingsInst->TrickleGroupSize,
	                   ElapsedTime,
	                   CachedSettingsInst->TrickleGroupSpawnDelay);

	LastTrickleSpawnTime = ElapsedTime;
}

void UTPSWaveSubsystem::TickBigWave()
{
	if (CurrentPhase != EWavePhase::BigWaveActive) { return; }
	if (BigWaveGroupsRemaining <= 0)
	{
		CurrentPhase = EWavePhase::Trickle;
		return;
	}

	if (ElapsedTime < NextBigWaveGroupTime) { return; }

	const int32 TotalCount = CachedSettingsInst->BigWaveBaseCount
	    + CurrentWaveLevel * CachedSettingsInst->BigWaveCountPerLevel;
	const int32 GroupCount = FMath::Max(CachedSettingsInst->BigWaveGroupCount, 1);
	const int32 PerGroup = TotalCount / GroupCount;

	ScheduleSpawnGroup(PerGroup, ElapsedTime, CachedSettingsInst->TrickleGroupSpawnDelay);
	--BigWaveGroupsRemaining;
	NextBigWaveGroupTime += CachedSettingsInst->BigWaveGroupInterval;
}

void UTPSWaveSubsystem::ScheduleSpawnGroup(int32 Count, float AtTime, float StaggerDelay)
{
	for (int32 i = 0; i < Count; ++i)
	{
		const UTPSEnemyTypeDataAsset* pType = SelectEnemyType();
		if (!ensure(pType)) { continue; }

		FWaveSpawnRequest Request;
		Request.Params = BuildSpawnParams(pType);
		Request.ScheduledTime = AtTime + static_cast<float>(i) * StaggerDelay;

		PendingSpawnQueue.Add(MoveTemp(Request));
	}
}

void UTPSWaveSubsystem::FlushReadySpawns()
{
	if (PendingSpawnQueue.Num() == 0) { return; }

	UEnemyManagerSubsystem* pEnemySub = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
	if (!ensure(pEnemySub)) { return; }

	int32 FlushedCount = 0;
	for (int32 i = PendingSpawnQueue.Num() - 1; i >= 0; --i)
	{
		if (PendingSpawnQueue[i].ScheduledTime <= ElapsedTime)
		{
			pEnemySub->QueueSpawn(PendingSpawnQueue[i].Params);
			PendingSpawnQueue.RemoveAtSwap(i);
			++FlushedCount;
		}
	}
	if (FlushedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Wave] FlushReadySpawns — Flushed %d, Remaining=%d"),
			FlushedCount, PendingSpawnQueue.Num());
	}
}

FEnemySpawnParams UTPSWaveSubsystem::BuildSpawnParams(const UTPSEnemyTypeDataAsset* Type) const
{
	FEnemySpawnParams Params;
	Params.Position       = PickRandomSpawnLocation();
	Params.MaxHealth      = Type->MaxHealth      * GetStatMultiplier(CachedSettingsInst->HealthMultiplierPerWave);
	Params.MaxSpeed       = Type->MaxSpeed       * GetStatMultiplier(CachedSettingsInst->SpeedMultiplierPerWave);
	Params.AttackDamage   = Type->AttackDamage   * GetStatMultiplier(CachedSettingsInst->DamageMultiplierPerWave);
	Params.AttackCooldown = Type->AttackCooldown;
	return Params;
}

void UTPSWaveSubsystem::CollectSpawnPoints()
{
	CachedSpawnPoints.Empty();

	UWorld* pWorld = GetWorld();
	if (!ensure(pWorld)) { return; }

	for (TActorIterator<ATPSEnemySpawnPoint> It(pWorld); It; ++It)
	{
		ATPSEnemySpawnPoint* pPoint = *It;
		if (pPoint && pPoint->bEnabled)
		{
			CachedSpawnPoints.Add(pPoint);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("WaveSubsystem: Collected %d spawn points"), CachedSpawnPoints.Num());
}

FVector UTPSWaveSubsystem::PickRandomSpawnLocation() const
{
	// 유효한 포인트만 필터
	TArray<ATPSEnemySpawnPoint*> ValidPoints;
	for (const TWeakObjectPtr<ATPSEnemySpawnPoint>& Weak : CachedSpawnPoints)
	{
		ATPSEnemySpawnPoint* pPoint = Weak.Get();
		if (pPoint && pPoint->bEnabled)
		{
			ValidPoints.Add(pPoint);
		}
	}

	if (ValidPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaveSubsystem: No valid spawn points — spawning at origin"));
		return FVector::ZeroVector;
	}

	ATPSEnemySpawnPoint* pChosen = ValidPoints[FMath::RandRange(0, ValidPoints.Num() - 1)];

	FVector Location = pChosen->GetSpawnLocation();

	// SpawnRadius > 0 이면 반경 내 랜덤 오프셋
	if (pChosen->SpawnRadius > 0.f)
	{
		const float Angle  = FMath::FRandRange(0.f, 2.f * PI);
		const float Radius = FMath::FRandRange(0.f, pChosen->SpawnRadius);
		Location += FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
	}

	return Location;
}

const UTPSEnemyTypeDataAsset* UTPSWaveSubsystem::SelectEnemyType() const
{
	int32 TotalWeight = 0;
	for (const TObjectPtr<UTPSEnemyTypeDataAsset>& pType : LoadedEnemyTypes)
	{
		if (pType && pType->MinWaveLevel <= CurrentWaveLevel)
		{
			TotalWeight += pType->SpawnWeight;
		}
	}

	if (TotalWeight <= 0) { return nullptr; }

	int32 Roll = FMath::RandRange(0, TotalWeight - 1);
	for (const TObjectPtr<UTPSEnemyTypeDataAsset>& pType : LoadedEnemyTypes)
	{
		if (!pType || pType->MinWaveLevel > CurrentWaveLevel) { continue; }

		Roll -= pType->SpawnWeight;
		if (Roll < 0)
		{
			return pType.Get();
		}
	}

	return nullptr;
}
