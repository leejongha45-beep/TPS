#include "Spawn/TPSPlayerSpawnSubsystem.h"
#include "Spawn/TPSPlayerStart.h"
#include "EngineUtils.h"

void UTPSPlayerSpawnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	CollectSpawnPoints(&InWorld);
}

void UTPSPlayerSpawnSubsystem::Deinitialize()
{
	CachedSpawnPoints.Empty();

	Super::Deinitialize();
}

// ──────────────────────────────────────────────
//  내부 — 수집
// ──────────────────────────────────────────────

void UTPSPlayerSpawnSubsystem::CollectSpawnPoints(UWorld* World)
{
	if (!ensure(World)) return;

	CachedSpawnPoints.Empty();

	for (TActorIterator<ATPSPlayerStart> It(World); It; ++It)
	{
		ATPSPlayerStart* pPoint = *It;
		if (ensure(pPoint))
		{
			CachedSpawnPoints.Add(pPoint);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayerSpawnSubsystem] Collected %d player spawn points"),
		CachedSpawnPoints.Num());
}

// ──────────────────────────────────────────────
//  쿼리
// ──────────────────────────────────────────────

TArray<ATPSPlayerStart*> UTPSPlayerSpawnSubsystem::GetActiveSpawnPoints() const
{
	TArray<ATPSPlayerStart*> Result;

	for (ATPSPlayerStart* pPoint : CachedSpawnPoints)
	{
		if (ensure(pPoint) && pPoint->IsActive())
		{
			Result.Add(pPoint);
		}
	}

	return Result;
}

// ──────────────────────────────────────────────
//  비활성화
// ──────────────────────────────────────────────

void UTPSPlayerSpawnSubsystem::DeactivateSpawnPoint(ATPSPlayerStart* InPoint)
{
	if (!ensure(InPoint)) return;
	if (!InPoint->IsActive()) return;

	InPoint->Deactivate();

	UE_LOG(LogTemp, Log, TEXT("[PlayerSpawnSubsystem] Deactivated player spawn: %s"),
		*InPoint->GetDisplayName().ToString());

	OnPlayerSpawnDeactivatedDelegate.Broadcast(InPoint);
}
