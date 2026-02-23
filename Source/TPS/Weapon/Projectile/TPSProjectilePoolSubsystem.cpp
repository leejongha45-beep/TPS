#include "TPSProjectilePoolSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "TPSProjectilePoolConfig.h"
#include "Weapon/Projectile/TPSProjectileBase.h"

DECLARE_LOG_CATEGORY_EXTERN(PoolLog, Log, All);
DEFINE_LOG_CATEGORY(PoolLog);

void UTPSProjectilePoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ① DataAsset 로드 (풀 설정)
	ConfigAsset = LoadObject<UTPSProjectilePoolConfig>(
		nullptr, TEXT("/Game/Assets/Data/DA_ProjectilePoolConfig.DA_ProjectilePoolConfig"));

	if (!ensure(ConfigAsset.Get()))
	{
		UE_LOG(PoolLog, Error, TEXT("[Initialize] DA_ProjectilePoolConfig not found. Create DataAsset at /Game/Assets/Data/DA_ProjectilePoolConfig"));
		return;
	}

	// ② 설정값 캐싱 + 풀 메모리 예약
	PoolSize = ConfigAsset->PoolSize;
	DeferredSpawnBatchSize = ConfigAsset->DeferredSpawnBatchSize;
	Pool.Reserve(PoolSize);
}

void UTPSProjectilePoolSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// ① 투사체 클래스 동기 로드 (SoftObjectPtr)
	if (ConfigAsset.Get() == nullptr || ConfigAsset->ProjectileClassPath.IsNull())
	{
		UE_LOG(PoolLog, Warning, TEXT("[OnWorldBeginPlay] ProjectileClassPath is not set. Pool will not be initialized."));
		return;
	}

	LoadedProjectileClass = ConfigAsset->ProjectileClassPath.LoadSynchronous();
	if (!ensure(LoadedProjectileClass.Get()))
	{
		UE_LOG(PoolLog, Error, TEXT("[OnWorldBeginPlay] Failed to load ProjectileClass from soft reference."));
		return;
	}

	// ② 초기 배치 스폰 (InitialSpawnCount만큼)
	SpawnProjectileBatch(ConfigAsset->InitialSpawnCount);

	UE_LOG(PoolLog, Log, TEXT("[OnWorldBeginPlay] Initial pool spawned: %d / %d"), TotalSpawnedCount, PoolSize);

	// ③ 나머지 → 프레임 분산 스폰 타이머 등록
	if (TotalSpawnedCount < PoolSize)
	{
		InWorld.GetTimerManager().SetTimer(
			DeferredSpawnTimerHandle, this, &UTPSProjectilePoolSubsystem::DeferredSpawn,
			0.016f, true);
	}
}

void UTPSProjectilePoolSubsystem::Deinitialize()
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

ATPSProjectileBase* UTPSProjectilePoolSubsystem::GetProjectile()
{
	// ① 풀에 여유분이 있으면 즉시 반환
	if (Pool.Num() > 0)
	{
		return Pool.Pop();
	}

	// ② 풀 고갈 시 긴급 스폰
	UE_LOG(PoolLog, Warning, TEXT("[GetProjectile] Pool exhausted! Emergency spawning projectile."));

	UWorld* pWorld = GetWorld();
	if (!ensure(pWorld)) return nullptr;
	if (!ensure(LoadedProjectileClass.Get())) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATPSProjectileBase* pProjectile = pWorld->SpawnActor<ATPSProjectileBase>(LoadedProjectileClass, FTransform::Identity, SpawnParams);
	if (ensure(pProjectile))
	{
		++TotalSpawnedCount;
	}

	return pProjectile;
}

void UTPSProjectilePoolSubsystem::ReturnProjectile(ATPSProjectileBase* InProjectile)
{
	if (!ensure(InProjectile)) return;

	Pool.Push(InProjectile);
}

void UTPSProjectilePoolSubsystem::SpawnProjectileBatch(int32 InCount)
{
	UWorld* pWorld = GetWorld();
	if (!ensure(pWorld)) return;
	if (!ensure(LoadedProjectileClass.Get())) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < InCount; ++i)
	{
		ATPSProjectileBase* pProjectile = pWorld->SpawnActor<ATPSProjectileBase>(
			LoadedProjectileClass, FTransform(FVector(0.f, 0.f, -10000.f)), SpawnParams);
		if (ensure(pProjectile))
		{
			Pool.Add(pProjectile);
			++TotalSpawnedCount;
		}
	}
}

void UTPSProjectilePoolSubsystem::DeferredSpawn()
{
	const int32 Remaining = PoolSize - TotalSpawnedCount;
	if (Remaining <= 0)
	{
		UWorld* pWorld = GetWorld();
		if (pWorld)
		{
			pWorld->GetTimerManager().ClearTimer(DeferredSpawnTimerHandle);
		}
		UE_LOG(PoolLog, Log, TEXT("[DeferredSpawn] Pool fully spawned: %d"), TotalSpawnedCount);
		return;
	}

	const int32 BatchCount = FMath::Min(DeferredSpawnBatchSize, Remaining);
	SpawnProjectileBatch(BatchCount);
}
