#include "ECS/System/FlowFieldSystem.h"
#include "Engine/World.h"
#include "NavigationSystem.h"

void TerrainCacheSystem::Build(FTerrainHeightCache& Cache, UWorld* World, const FVector& MapCenter)
{
	if (!World) { return; }

	Cache.OriginX = MapCenter.X - FTerrainHeightCache::HalfExtent;
	Cache.OriginY = MapCenter.Y - FTerrainHeightCache::HalfExtent;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys)
	{
		UE_LOG(LogTemp, Error, TEXT("[TerrainCache] NavigationSystem not found"));
		return;
	}

	const float ProjectionExtent = 50000.f;

	for (int32 i = 0; i < FTerrainHeightCache::TotalCells; ++i)
	{
		const FVector2f CellCenter = Cache.IndexToWorld(i);
		const FVector QueryLocation(CellCenter.X, CellCenter.Y, MapCenter.Z);

		FNavLocation NavResult;
		const bool bProjected = NavSys->ProjectPointToNavigation(
			QueryLocation, NavResult, FVector(0.f, 0.f, ProjectionExtent));

		if (bProjected)
		{
			Cache.Heights[i] = NavResult.Location.Z;
		}
		else
		{
			Cache.Blocked[i] = true;
			Cache.Heights[i] = FTerrainHeightCache::InvalidHeight;
		}
	}

	Cache.bReady = true;

	int32 BlockedCount = 0;
	for (int32 i = 0; i < FTerrainHeightCache::TotalCells; ++i)
	{
		if (Cache.Blocked[i]) { ++BlockedCount; }
	}
	UE_LOG(LogTemp, Warning, TEXT("[TerrainCache] Built — Blocked=%d/%d"),
		BlockedCount, FTerrainHeightCache::TotalCells);
}
