#include "TPSProjectilePoolSubsystem.h"
#include "TPSProjectilePoolConfig.h"
#include "Weapon/Projectile/TPSProjectileBase.h"

DECLARE_LOG_CATEGORY_EXTERN(PoolLog, Log, All);
DEFINE_LOG_CATEGORY(PoolLog);

void UTPSProjectilePoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ConfigAsset = LoadObject<UTPSProjectilePoolConfig>(
		nullptr, TEXT("/Game/Assets/Data/DA_ProjectilePoolConfig.DA_ProjectilePoolConfig"));

	if (!ensure(ConfigAsset))
	{
		UE_LOG(PoolLog, Error, TEXT("[Initialize] DA_ProjectilePoolConfig not found. Create DataAsset at /Game/Assets/Data/DA_ProjectilePoolConfig"));
		return;
	}

	PoolSize = ConfigAsset->PoolSize;
	DeferredSpawnBatchSize = ConfigAsset->DeferredSpawnBatchSize;
	Pool.Reserve(PoolSize);
}

void UTPSProjectilePoolSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!ConfigAsset || ConfigAsset->ProjectileClassPath.IsNull())
	{
		UE_LOG(PoolLog, Warning, TEXT("[OnWorldBeginPlay] ProjectileClassPath is not set. Pool will not be initialized."));
		return;
	}

	LoadedProjectileClass = ConfigAsset->ProjectileClassPath.LoadSynchronous();
	if (!ensure(LoadedProjectileClass))
	{
		UE_LOG(PoolLog, Error, TEXT("[OnWorldBeginPlay] Failed to load ProjectileClass from soft reference."));
		return;
	}

	SpawnProjectileBatch(ConfigAsset->InitialSpawnCount);

	UE_LOG(PoolLog, Log, TEXT("[OnWorldBeginPlay] Initial pool spawned: %d / %d"), TotalSpawnedCount, PoolSize);

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
	if (Pool.Num() > 0)
	{
		return Pool.Pop();
	}

	UE_LOG(PoolLog, Warning, TEXT("[GetProjectile] Pool exhausted! Emergency spawning projectile."));

	UWorld* pWorld = GetWorld();
	if (!ensure(pWorld)) return nullptr;
	if (!ensure(LoadedProjectileClass)) return nullptr;

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
	if (!ensure(LoadedProjectileClass)) return;

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
