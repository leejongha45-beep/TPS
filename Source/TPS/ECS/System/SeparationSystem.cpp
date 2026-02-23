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

/** ② Write: 분리력 블렌딩된 속도를 CMovement에 쓰기 */
void Write(CMovement& OutMovement, const FVector& BlendedVelocity)
{
	OutMovement.Velocity = BlendedVelocity;
}

/** ③ PushToPrev: 갱신된 CMovement → CMovementPrev 복사 */
void PushToPrev(CMovementPrev& OutPrev, const CMovement& InCurrent)
{
	OutPrev.Velocity = InCurrent.Velocity;
	OutPrev.MaxSpeed = InCurrent.MaxSpeed;
}

void SeparationSystem::Tick(entt::registry& Registry, const FVector& PlayerPosition)
{
	// ── Step A: Cull + Grid Build (GT 단일) ──
	FSpatialGrid Grid;
	Grid.Build(Registry, ECSConstants::SeparationRadius,
	           PlayerPosition, ECSConstants::SeparationCullingRadiusSq);

	const int32 Count = Grid.Entities.Num();
	if (Count <= 1) { return; }

	// ── Step B: Force Compute (ParallelFor) ──
	auto View = Registry.view<CMovement, CMovementPrev>();

	ParallelFor(Count, [&Grid, &View](int32 Index)
	{
		const FVector& MyPos = Grid.Positions[Index];
		const int32 MyCellX = FMath::FloorToInt32(MyPos.X / Grid.CellSize);
		const int32 MyCellY = FMath::FloorToInt32(MyPos.Y / Grid.CellSize);

		FVector SeparationForce = FVector::ZeroVector;
		int32 NeighborCount = 0;

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
					float Strength;

					// ★ 퇴화 케이스: 거의 동일 위치 → 랜덤 방향 지터
					if (DistSq < KINDA_SMALL_NUMBER)
					{
						Delta = FVector(FMath::FRandRange(-1.f, 1.f),
						                FMath::FRandRange(-1.f, 1.f), 0.f).GetSafeNormal();
						Strength = 1.f;
					}
					else if (DistSq >= ECSConstants::SeparationRadiusSq)
					{
						continue;
					}
					else
					{
						const float Dist = FMath::Sqrt(DistSq);
						Strength = 1.f - (Dist / ECSConstants::SeparationRadius);
						Delta /= Dist;
					}

					SeparationForce += Delta * Strength;
					++NeighborCount;
				}
			}
		}

		if (NeighborCount == 0) { return; }

		// 평균 → 정규화 → 최대 분리력 클램프
		SeparationForce /= static_cast<float>(NeighborCount);
		SeparationForce = SeparationForce.GetClampedToMaxSize(1.f) * ECSConstants::MaxSeparationForce;

		// 기존 추격 속도 + 분리력 블렌딩 → MaxSpeed 클램프
		const entt::entity Entity = Grid.Entities[Index];
		CMovement& Movement = View.get<CMovement>(Entity);
		const float MaxSpeed = View.get<CMovementPrev>(Entity).MaxSpeed;

		const FVector BlendedVelocity = (Movement.Velocity + SeparationForce * ECSConstants::SeparationWeight)
		                                .GetClampedToMaxSize(MaxSpeed);

		// ② Write + ③ PushToPrev
		Write(Movement, BlendedVelocity);
		PushToPrev(View.get<CMovementPrev>(Entity), Movement);
	});
}
