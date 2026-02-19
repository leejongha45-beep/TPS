#include "Enemy/Pool/TPSEnemyActorPoolSubsystem.h"
#include "Enemy/Pool/TPSEnemyActorPoolConfig.h"
#include "Enemy/Actor/TPSEnemyPawnBase.h"

DECLARE_LOG_CATEGORY_EXTERN(EnemyPoolLog, Log, All);
DEFINE_LOG_CATEGORY(EnemyPoolLog);

void UTPSEnemyActorPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ① DataAsset 로드 (풀 설정)
	ConfigAsset = LoadObject<UTPSEnemyActorPoolConfig>(
		nullptr, TEXT("/Game/Assets/Data/DA_EnemyActorPoolConfig.DA_EnemyActorPoolConfig"));

	if (!ensure(ConfigAsset))
	{
		UE_LOG(EnemyPoolLog, Error, TEXT("[Initialize] DA_EnemyActorPoolConfig not found. Create DataAsset at /Game/Assets/Data/DA_EnemyActorPoolConfig"));
		return;
	}

	// ② 설정값 캐싱 + 풀 메모리 예약
	PoolSize = ConfigAsset->PoolSize;
	DeferredSpawnBatchSize = ConfigAsset->DeferredSpawnBatchSize;
	Pool.Reserve(PoolSize);
}

void UTPSEnemyActorPoolSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// ① 적 폰 클래스 동기 로드 (SoftClassPtr)
	if (!ConfigAsset || ConfigAsset->EnemyPawnClassPath.IsNull())
	{
		UE_LOG(EnemyPoolLog, Warning, TEXT("[OnWorldBeginPlay] EnemyPawnClassPath is not set. Pool will not be initialized."));
		return;
	}

	LoadedEnemyClass = ConfigAsset->EnemyPawnClassPath.LoadSynchronous();
	if (!ensure(LoadedEnemyClass))
	{
		UE_LOG(EnemyPoolLog, Error, TEXT("[OnWorldBeginPlay] Failed to load EnemyPawnClass from soft reference."));
		return;
	}

	// ② 초기 배치 스폰 (InitialSpawnCount만큼)
	SpawnEnemyBatch(ConfigAsset->InitialSpawnCount);

	UE_LOG(EnemyPoolLog, Log, TEXT("[OnWorldBeginPlay] Initial pool spawned: %d / %d"), TotalSpawnedCount, PoolSize);

	// ③ 나머지 → 프레임 분산 스폰 타이머 등록
	if (TotalSpawnedCount < PoolSize)
	{
		InWorld.GetTimerManager().SetTimer(
			DeferredSpawnTimerHandle, this, &UTPSEnemyActorPoolSubsystem::DeferredSpawn,
			0.016f, true);
	}
}

void UTPSEnemyActorPoolSubsystem::Deinitialize()
{
	UWorld* pWorld = GetWorld();
	if (pWorld)
	{
		pWorld->GetTimerManager().ClearTimer(DeferredSpawnTimerHandle);
	}

	Pool.Empty();
	TotalSpawnedCount = 0;

	Super::Deinitialize();
}

ATPSEnemyPawnBase* UTPSEnemyActorPoolSubsystem::GetEnemy()
{
	// ① 풀에 여유분이 있으면 즉시 반환
	if (Pool.Num() > 0)
	{
		return Pool.Pop();
	}

	// ② 프레임 카운트 리셋 (새 프레임 진입 시)
	const uint64 CurrentFrame = GFrameCounter;
	if (CurrentFrame != LastExpansionFrameNumber)
	{
		FrameExpansionCount = 0;
		LastExpansionFrameNumber = CurrentFrame;
	}

	// ③ 프레임당 확장 상한 체크 (10회 × 10마리 = 100마리/프레임)
	if (FrameExpansionCount >= MaxExpansionPerFrame)
	{
		return nullptr;  // LODProcessor가 ISM 폴백
	}

	// ④ 긴급 배치 확장
	UE_LOG(EnemyPoolLog, Warning, TEXT("[GetEnemy] Pool exhausted — expanding +%d (frame %llu, expansion %d/%d)"),
		ExpansionBatchSize, CurrentFrame, FrameExpansionCount + 1, MaxExpansionPerFrame);

	SpawnEnemyBatch(ExpansionBatchSize);
	++FrameExpansionCount;

	// ⑤ 확장 성공 시 반환
	if (Pool.Num() > 0)
	{
		return Pool.Pop();
	}

	return nullptr;
}

void UTPSEnemyActorPoolSubsystem::ReturnEnemy(ATPSEnemyPawnBase* InEnemy)
{
	if (!ensure(InEnemy)) return;

	Pool.Push(InEnemy);
}

void UTPSEnemyActorPoolSubsystem::SpawnEnemyBatch(int32 InCount)
{
	UWorld* pWorld = GetWorld();
	if (!ensure(pWorld)) return;
	if (!ensure(LoadedEnemyClass)) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < InCount; ++i)
	{
		ATPSEnemyPawnBase* pEnemy = pWorld->SpawnActor<ATPSEnemyPawnBase>(
			LoadedEnemyClass, FTransform(FVector(-10000.f, 0.f, 0.f)), SpawnParams);
		if (ensure(pEnemy))
		{
			Pool.Add(pEnemy);
			++TotalSpawnedCount;
		}
	}
}

void UTPSEnemyActorPoolSubsystem::DeferredSpawn()
{
	const int32 Remaining = PoolSize - TotalSpawnedCount;
	if (Remaining <= 0)
	{
		UWorld* pWorld = GetWorld();
		if (pWorld)
		{
			pWorld->GetTimerManager().ClearTimer(DeferredSpawnTimerHandle);
		}
		UE_LOG(EnemyPoolLog, Log, TEXT("[DeferredSpawn] Pool fully spawned: %d"), TotalSpawnedCount);
		return;
	}

	const int32 BatchCount = FMath::Min(DeferredSpawnBatchSize, Remaining);
	SpawnEnemyBatch(BatchCount);
}
