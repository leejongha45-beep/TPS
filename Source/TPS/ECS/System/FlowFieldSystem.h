#pragma once

#include "Containers/Array.h"
#include "Containers/BitArray.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"

/**
 * FFlowField — SoA 2D 그리드 기반 Flow Field
 *
 * BeginPlay 1회:
 *   [GT] Pass 1 라인트레이스 → Heights + Blocked
 *   [Worker] Pass 2 경사각 절벽 판정 → CliffEdges
 *   [Worker] Pass 3 BFS → CostField + Directions
 *
 * 런타임:
 *   AISystem ParallelFor에서 LookupDirection/LookupHeight 읽기 전용 접근
 */
struct FFlowField
{
	// ── 그리드 설정 ──
	static constexpr int32 GridSize = 128;
	static constexpr int32 TotalCells = GridSize * GridSize;   // 16384
	static constexpr float CellSize = 200.f;                   // 200uu = 2m
	static constexpr float HalfExtent = GridSize * CellSize * 0.5f;  // 12800uu
	static constexpr float MaxWalkableAngleRad = 0.6108f;      // 35° in radians
	static constexpr float InvalidHeight = -1e10f;

	// ── SoA 데이터 ── (캐시 라인 최적)
	TArray<FVector2f> Directions;   // [TotalCells] 정규화된 2D 방향
	TArray<float>     Heights;      // [TotalCells] 지면 Z값
	TBitArray<>       Blocked;      // [TotalCells] 차단 여부
	TArray<uint8>     CliffEdges;   // [TotalCells] 8비트: 방향별 절벽
	TArray<uint16>    CostField;    // [TotalCells] BFS 비용

	// ── 그리드 원점 (좌하단 월드 좌표) ──
	float OriginX = 0.f;
	float OriginY = 0.f;

	// ── 상태 ──
	uint8 bReady : 1 = false;

	/** 배열 할당 */
	void Initialize()
	{
		Directions.SetNumZeroed(TotalCells);
		Heights.SetNumUninitialized(TotalCells);
		for (int32 i = 0; i < TotalCells; ++i) { Heights[i] = InvalidHeight; }
		Blocked.Init(false, TotalCells);
		CliffEdges.SetNumZeroed(TotalCells);
		CostField.SetNumUninitialized(TotalCells);
		FMemory::Memset(CostField.GetData(), 0xFF, TotalCells * sizeof(uint16));
	}

	/** 월드 좌표 → 그리드 인덱스 (범위 밖이면 INDEX_NONE) */
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

	/** 그리드 인덱스 → 셀 중심 월드 좌표 */
	FORCEINLINE FVector2f IndexToWorld(int32 Index) const
	{
		const int32 CellX = Index % GridSize;
		const int32 CellY = Index / GridSize;
		return FVector2f(
			OriginX + (CellX + 0.5f) * CellSize,
			OriginY + (CellY + 0.5f) * CellSize
		);
	}

	/** 위치로 방향 조회 (범위 밖/미도달이면 ZeroVector) */
	FORCEINLINE FVector2f LookupDirection(float WorldX, float WorldY) const
	{
		if (!bReady) { return FVector2f::ZeroVector; }
		const int32 Idx = WorldToIndex(WorldX, WorldY);
		if (Idx == INDEX_NONE) { return FVector2f::ZeroVector; }
		return Directions[Idx];
	}

	/** 위치로 지면 높이 조회 (범위 밖이면 InvalidHeight) */
	FORCEINLINE float LookupHeight(float WorldX, float WorldY) const
	{
		if (!bReady) { return InvalidHeight; }
		const int32 Idx = WorldToIndex(WorldX, WorldY);
		if (Idx == INDEX_NONE) { return InvalidHeight; }
		return Heights[Idx];
	}
};

/**
 * FlowFieldSystem — 지형 캐시 + BFS 빌드
 *
 * [GT] BuildTerrainCache: 라인트레이스 → Heights + Blocked (Pass 1)
 * [Worker] BuildCliffEdgesAndBFS: 절벽 엣지 + BFS (Pass 2+3)
 */
namespace FlowFieldSystem
{
	/** Pass 1 [GameThread]: 라인트레이스로 Heights + Blocked 캐싱 */
	void BuildTerrainCache(FFlowField& FlowField, class UWorld* World, const FVector& MapCenter);

	/** Pass 2+3 [Worker-safe]: 절벽 엣지 계산 + BFS → Directions */
	void BuildCliffEdgesAndBFS(FFlowField& FlowField, int32 GoalIndex);
}
