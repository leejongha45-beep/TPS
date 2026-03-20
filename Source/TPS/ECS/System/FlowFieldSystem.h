#pragma once

#include "Containers/Array.h"
#include "Containers/BitArray.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"

/**
 * FTerrainHeightCache — NavMesh 기반 지형 높이 캐시
 *
 * BeginPlay 1회:
 *   [GT] NavMesh ProjectPoint → Heights + Blocked
 *
 * 런타임:
 *   MovementSystem에서 LookupHeight 읽기 전용 접근 (Z 보정)
 */
struct FTerrainHeightCache
{
	// ── 그리드 설정 ──
	static constexpr int32 GridSize = 1000;
	static constexpr int32 TotalCells = GridSize * GridSize;
	static constexpr float CellSize = 200.f;
	static constexpr float HalfExtent = GridSize * CellSize * 0.5f;
	static constexpr float InvalidHeight = -1e10f;

	// ── SoA 데이터 ──
	TArray<float> Heights;      // [TotalCells] 지면 Z값
	TBitArray<>   Blocked;      // [TotalCells] 차단 여부

	// ── 그리드 원점 ──
	float OriginX = 0.f;
	float OriginY = 0.f;

	// ── 상태 ──
	uint8 bReady : 1 = false;

	void Initialize()
	{
		Heights.SetNumUninitialized(TotalCells);
		for (int32 i = 0; i < TotalCells; ++i) { Heights[i] = InvalidHeight; }
		Blocked.Init(false, TotalCells);
	}

	FORCEINLINE int32 WorldToIndex(float WorldX, float WorldY) const
	{
		const int32 CellX = FMath::FloorToInt32((WorldX - OriginX) / CellSize);
		const int32 CellY = FMath::FloorToInt32((WorldY - OriginY) / CellSize);
		if (CellX < 0 || CellX >= GridSize || CellY < 0 || CellY >= GridSize)
		{
			return INDEX_NONE;
		}
		return CellY * GridSize + CellX;
	}

	FORCEINLINE FVector2f IndexToWorld(int32 Index) const
	{
		const int32 CellX = Index % GridSize;
		const int32 CellY = Index / GridSize;
		return FVector2f(
			OriginX + (CellX + 0.5f) * CellSize,
			OriginY + (CellY + 0.5f) * CellSize
		);
	}

	/** 위치로 지면 높이 조회 — 바이리니어 보간 */
	FORCEINLINE float LookupHeight(float WorldX, float WorldY) const
	{
		if (!bReady) { return InvalidHeight; }

		const float FX = (WorldX - OriginX) / CellSize - 0.5f;
		const float FY = (WorldY - OriginY) / CellSize - 0.5f;

		const int32 X0 = FMath::FloorToInt32(FX);
		const int32 Y0 = FMath::FloorToInt32(FY);
		const int32 X1 = X0 + 1;
		const int32 Y1 = Y0 + 1;

		if (X0 < 0 || X1 >= GridSize || Y0 < 0 || Y1 >= GridSize) { return InvalidHeight; }

		const float H00 = Heights[Y0 * GridSize + X0];
		const float H10 = Heights[Y0 * GridSize + X1];
		const float H01 = Heights[Y1 * GridSize + X0];
		const float H11 = Heights[Y1 * GridSize + X1];

		if (H00 <= InvalidHeight || H10 <= InvalidHeight ||
		    H01 <= InvalidHeight || H11 <= InvalidHeight) { return InvalidHeight; }

		const float TX = FX - static_cast<float>(X0);
		const float TY = FY - static_cast<float>(Y0);

		return H00 * (1.f - TX) * (1.f - TY)
		     + H10 * TX * (1.f - TY)
		     + H01 * (1.f - TX) * TY
		     + H11 * TX * TY;
	}
};

/** NavMesh 기반 지형 높이 빌드 */
namespace TerrainCacheSystem
{
	/** [GameThread] NavMesh ProjectPoint → Heights + Blocked */
	void Build(FTerrainHeightCache& Cache, class UWorld* World, const FVector& MapCenter);
}
