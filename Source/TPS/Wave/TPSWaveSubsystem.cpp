// TPSWaveSubsystem.cpp

#include "Wave/TPSWaveSubsystem.h"
#include "Wave/TPSWaveSettings.h"
#include "Engine/World.h"
#include "ECS/Data/TPSEnemyTypeDataAsset.h"
#include "ECS/Scheduler/EnemyManagerSubsystem.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"

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

	// ① AllyBase 위치 캐시 (스폰 링 중심점)
	UTPSTargetSubsystem* pTargetSub = GetWorld()->GetSubsystem<UTPSTargetSubsystem>();
	if (ensure(pTargetSub))
	{
		AllyBaseLocation = pTargetSub->GetAllyBaseLocation();
	}

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
	if (!ensure(pEnemySub)) { return; }

	UHierarchicalInstancedStaticMeshComponent* pHISM = pEnemySub->GetHISM();
	if (!ensure(pHISM)) { return; }

	const int32 CurrentCount = pHISM->GetInstanceCount() + PendingSpawnQueue.Num();
	if (CurrentCount >= CachedSettingsInst->MaxEnemyCount) { return; }

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

	for (int32 i = PendingSpawnQueue.Num() - 1; i >= 0; --i)
	{
		if (PendingSpawnQueue[i].ScheduledTime <= ElapsedTime)
		{
			pEnemySub->QueueSpawn(PendingSpawnQueue[i].Params);
			PendingSpawnQueue.RemoveAtSwap(i);
		}
	}
}

FEnemySpawnParams UTPSWaveSubsystem::BuildSpawnParams(const UTPSEnemyTypeDataAsset* Type) const
{
	FEnemySpawnParams Params;
	Params.Position       = CalculateSpawnPosition();
	Params.MaxHealth      = Type->MaxHealth      * GetStatMultiplier(CachedSettingsInst->HealthMultiplierPerWave);
	Params.MaxSpeed       = Type->MaxSpeed       * GetStatMultiplier(CachedSettingsInst->SpeedMultiplierPerWave);
	Params.AttackDamage   = Type->AttackDamage   * GetStatMultiplier(CachedSettingsInst->DamageMultiplierPerWave);
	Params.AttackCooldown = Type->AttackCooldown;
	return Params;
}

FVector UTPSWaveSubsystem::CalculateSpawnPosition() const
{
	const float Angle  = FMath::FRandRange(0.f, 2.f * PI);
	const float Radius = FMath::FRandRange(CachedSettingsInst->SpawnRingMinRadius,
	                                       CachedSettingsInst->SpawnRingMaxRadius);

	return AllyBaseLocation + FVector(FMath::Cos(Angle) * Radius,
	                                  FMath::Sin(Angle) * Radius,
	                                  0.f);
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
