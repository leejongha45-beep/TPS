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

	CachedSettings = GetDefault<UTPSWaveSettings>();

	if (ensure(CachedSettings.Get()))
	{
		CachedEnemyType = CachedSettings->EnemyType.LoadSynchronous();
		ensure(CachedEnemyType.Get());
	}
}

void UTPSWaveSubsystem::Deinitialize()
{
	StopWaveSystem();
	CachedEnemyType = nullptr;
	Super::Deinitialize();
}

TStatId UTPSWaveSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTPSWaveSubsystem, STATGROUP_Tickables);
}

void UTPSWaveSubsystem::StartWaveSystem()
{
	if (bIsActive) { return; }

	CollectSpawnPoints();

	UE_LOG(LogTemp, Warning, TEXT("[Wave] StartWaveSystem — SpawnPoints=%d, EnemyType=%s"),
		CachedSpawnPoints.Num(), CachedEnemyType.Get() ? *CachedEnemyType->GetName() : TEXT("None"));

	bIsActive = true;
	CurrentPhase = EWavePhase::Trickle;
	CurrentWaveLevel = 0;
	ElapsedTime = 0.f;
	LastTrickleTime = 0.f;
	LastBigWaveTime = 0.f;
}

void UTPSWaveSubsystem::StopWaveSystem()
{
	bIsActive = false;
	CurrentPhase = EWavePhase::Idle;
}

void UTPSWaveSubsystem::Tick(float DeltaTime)
{
	if (!bIsActive) { return; }
	if (!ensure(CachedSettings.Get())) { return; }

	ElapsedTime += DeltaTime;

	// ── 빅웨이브 주기 도달 → Alert 전환 ──
	const float TimeSinceLastBigWave = ElapsedTime - LastBigWaveTime;
	if (CurrentPhase == EWavePhase::Trickle
		&& TimeSinceLastBigWave >= CachedSettings->BigWavePeriod)
	{
		++CurrentWaveLevel;
		CurrentPhase = EWavePhase::BigWaveAlert;
		LastBigWaveTime = ElapsedTime;

		UE_LOG(LogTemp, Warning, TEXT("[Wave] BigWaveAlert — Level=%d"), CurrentWaveLevel);
	}

	// ── Alert 시간 경과 → 일괄 스폰 → Trickle 복귀 ──
	if (CurrentPhase == EWavePhase::BigWaveAlert)
	{
		const float AlertElapsed = ElapsedTime - LastBigWaveTime;
		if (AlertElapsed >= CachedSettings->BigWaveAlertDuration)
		{
			CurrentPhase = EWavePhase::BigWaveActive;

			const int32 TotalCount = CachedSettings->BigWaveBaseCount
				+ CurrentWaveLevel * CachedSettings->BigWaveCountPerLevel;

			UE_LOG(LogTemp, Warning, TEXT("[Wave] BigWaveActive — Spawning %d"), TotalCount);

			SpawnBatch(TotalCount);

			CurrentPhase = EWavePhase::Trickle;
		}
	}

	// ── 트리클 (항상 동작) ──
	TickTrickleSpawn();
}

void UTPSWaveSubsystem::TickTrickleSpawn()
{
	if (ElapsedTime - LastTrickleTime < CachedSettings->TrickleSpawnInterval) { return; }

	// MaxEnemyCount 캡 체크
	UEnemyManagerSubsystem* pEnemySub = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
	if (!pEnemySub) { return; }

	UInstancedStaticMeshComponent* pHISM = pEnemySub->GetHISM();
	if (!pHISM) { return; }

	const int32 CurrentCount = pHISM->GetInstanceCount();
	if (CurrentCount >= CachedSettings->MaxEnemyCount) { return; }

	SpawnBatch(CachedSettings->TrickleGroupSize);
	LastTrickleTime = ElapsedTime;
}

void UTPSWaveSubsystem::SpawnBatch(int32 Count)
{
	UEnemyManagerSubsystem* pEnemySub = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
	if (!ensure(pEnemySub)) { return; }
	if (!CachedEnemyType.Get()) { return; }

	for (int32 i = 0; i < Count; ++i)
	{
		pEnemySub->QueueSpawn(BuildSpawnParams(CachedEnemyType.Get()));
	}
}

FEnemySpawnParams UTPSWaveSubsystem::BuildSpawnParams(const UTPSEnemyTypeDataAsset* Type) const
{
	FEnemySpawnParams Params;
	Params.Position       = PickRandomSpawnLocation();
	Params.MaxHealth      = Type->MaxHealth;
	Params.MaxSpeed       = Type->MaxSpeed;
	Params.AttackDamage   = Type->AttackDamage;
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
}

FVector UTPSWaveSubsystem::PickRandomSpawnLocation() const
{
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
		UE_LOG(LogTemp, Warning, TEXT("[Wave] No valid spawn points — spawning at origin"));
		return FVector::ZeroVector;
	}

	ATPSEnemySpawnPoint* pChosen = ValidPoints[FMath::RandRange(0, ValidPoints.Num() - 1)];
	FVector Location = pChosen->GetSpawnLocation();

	if (pChosen->SpawnRadius > 0.f)
	{
		const float Angle  = FMath::FRandRange(0.f, 2.f * PI);
		const float Radius = FMath::FRandRange(0.f, pChosen->SpawnRadius);
		Location += FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
	}

	return Location;
}

