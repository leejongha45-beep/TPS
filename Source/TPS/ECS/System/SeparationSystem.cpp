#include "ECS/System/SeparationSystem.h"
#include "ECS/Component/Components.h"
#include "Async/ParallelFor.h"

// ── FSpatialGrid ──

void FSpatialGrid::Build(entt::registry& Registry, float InCellSize,
                         const FVector& PlayerPosition, float CullingRadiusSq)
{
	CellSize = InCellSize;
	Cells.Reset();
	Positions.Reset();
	Entities.Reset();

	auto View = Registry.view<CTransformPrev, CEnemyStatePrev>();

	for (auto Entity : View)
	{
		const EEnemyState CachedState = View.get<CEnemyStatePrev>(Entity).State;

		// Dying/Dead만 제외 — Idle, Moving, AttackCooldown, AttackReady, Attacking 전부 포함
		if (CachedState == EEnemyState::Dying || CachedState == EEnemyState::Dead) { continue; }

		const FVector& Pos = View.get<CTransformPrev>(Entity).Position;

		// ① 플레이어 반경 컬링
		if ((Pos - PlayerPosition).SizeSquared() > CullingRadiusSq) { continue; }

		// ② 인덱스 등록
		const int32 Index = Positions.Num();
		Positions.Add(Pos);
		Entities.Add(Entity);

		// ③ 셀에 등록
		const int32 CellX = FMath::FloorToInt32(Pos.X / CellSize);
		const int32 CellY = FMath::FloorToInt32(Pos.Y / CellSize);
		Cells.FindOrAdd(MakeCellKey(CellX, CellY)).Add(Index);
	}
}

// ── SeparationSystem ──


void SeparationSystem::Tick(entt::registry& Registry, const FVector& PlayerPosition)
{
	// ── Step A: Cull + Grid Build (GT 단일) ──
	FSpatialGrid Grid;
	Grid.Build(Registry, ECSConstants::SeparationRadius,
	           PlayerPosition, ECSConstants::SeparationCullingRadiusSq);

	const int32 Count = Grid.Entities.Num();
	if (Count <= 1) { return; }

	// ── Step B: Hard Push (ParallelFor) — 위치 직접 보정 ──
	auto View = Registry.view<CTransform, CTransformPrev>();

	ParallelFor(Count, [&Grid, &View](int32 Index)
	{
		const FVector& MyPos = Grid.Positions[Index];
		const int32 MyCellX = FMath::FloorToInt32(MyPos.X / Grid.CellSize);
		const int32 MyCellY = FMath::FloorToInt32(MyPos.Y / Grid.CellSize);

		FVector Push = FVector::ZeroVector;

		// 9셀 탐색 (자기 셀 + 인접 8셀)
		for (int32 dx = -1; dx <= 1; ++dx)
		{
			for (int32 dy = -1; dy <= 1; ++dy)
			{
				const int64 CellKey = FSpatialGrid::MakeCellKey(MyCellX + dx, MyCellY + dy);
				const TArray<int32>* pNeighbors = Grid.Cells.Find(CellKey);
				if (!pNeighbors) { continue; }

				for (const int32 OtherIndex : *pNeighbors)
				{
					if (OtherIndex == Index) { continue; }

					FVector Delta = MyPos - Grid.Positions[OtherIndex];
					const float DistSq = Delta.SizeSquared();

					// ★ 퇴화 케이스: 동일 위치 → 인덱스 기반 결정론적 방향
					if (DistSq < 1.f)
					{
						const float Angle = static_cast<float>((Index * 2654435761u) ^ (OtherIndex * 340573321u)) * (UE_TWO_PI / 4294967296.f);
						Delta = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
						Push += Delta * ECSConstants::SeparationRadius;
					}
					else if (DistSq < ECSConstants::SeparationRadiusSq)
					{
						const float Dist = FMath::Sqrt(DistSq);
						const float Penetration = ECSConstants::SeparationRadius - Dist;
						Push += (Delta / Dist) * (Penetration * 0.5f);
					}
				}
			}
		}

		Push.Z = 0.f;
		if (Push.IsNearlyZero()) { return; }

		Push = Push.GetClampedToMaxSize(ECSConstants::MaxSeparationForce);

		// ② Write — Current에 쓰기
		const entt::entity Entity = Grid.Entities[Index];
		View.get<CTransform>(Entity).Position += Push;

		// ③ PushToPrev — Current → Prev
		View.get<CTransformPrev>(Entity).Position = View.get<CTransform>(Entity).Position;
	});
}
