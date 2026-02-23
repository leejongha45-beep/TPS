#pragma once

#include <entt/entt.hpp>
#include "Math/Vector.h"

/**
 * 공간 해시 그리드 — 이웃 탐색 O(1) 셀 룩업
 * - CellSize = SeparationRadius (같은 셀 + 인접 8셀만 탐색)
 * - Build 후 읽기 전용 → ParallelFor 안전
 */
struct FSpatialGrid
{
	/** 그리드 초기화 + Moving Entity 수집 + 셀 등록
	 * @param Registry        ECS 레지스트리
	 * @param InCellSize      셀 크기 (= SeparationRadius)
	 * @param PlayerPosition  플레이어 위치 (컬링 기준)
	 * @param CullingRadiusSq 컬링 반경 제곱
	 */
	void Build(entt::registry& Registry, float InCellSize,
	           const FVector& PlayerPosition, float CullingRadiusSq);

	/** 2D 셀 좌표 → int64 키 패킹 */
	static FORCEINLINE int64 MakeCellKey(int32 CellX, int32 CellY)
	{
		return (static_cast<int64>(CellX) << 32) | static_cast<uint32>(CellY);
	}

	float CellSize = 0.f;
	TMap<int64, TArray<int32>> Cells;
	TArray<FVector> Positions;
	TArray<entt::entity> Entities;
};

/**
 * 분리 시스템 — Entity 겹침 방지
 *
 * [GameThread] Phase 3.5 — AISystem 이후, DeathSystem 이전
 * - Step A (GT 단일): Cull + Grid Build — 플레이어 반경 내 Moving Entity만 수집
 * - Step B (ParallelFor): 이웃 탐색 + Boids 분리력 블렌딩
 *
 * Read:  CTransformPrev.Position, CEnemyStatePrev.State, CMovement.Velocity
 * Write: CMovement.Velocity (분리력 보정) + PushToPrev → CMovementPrev
 */
namespace SeparationSystem
{
	void Tick(entt::registry& Registry, const FVector& PlayerPosition);
};
