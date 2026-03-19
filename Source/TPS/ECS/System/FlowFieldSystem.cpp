#include "ECS/System/FlowFieldSystem.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"

namespace
{

// ── 8방향 오프셋 (직교 4 + 대각 4) ──
static constexpr int32 OffX[8] = { 1, -1,  0,  0,  1, -1,  1, -1 };
static constexpr int32 OffY[8] = { 0,  0,  1, -1,  1,  1, -1, -1 };

// 직교(0~3): CellSize, 대각(4~7): CellSize * sqrt(2)
static constexpr float HorizDist[8] = {
	FFlowField::CellSize, FFlowField::CellSize,
	FFlowField::CellSize, FFlowField::CellSize,
	FFlowField::CellSize * 1.41421356f, FFlowField::CellSize * 1.41421356f,
	FFlowField::CellSize * 1.41421356f, FFlowField::CellSize * 1.41421356f
};

/** Pass 2: 경사각 기반 절벽 엣지 계산 (순수 수학, Worker-safe) */
void BuildCliffEdges(FFlowField& FF)
{
	for (int32 i = 0; i < FFlowField::TotalCells; ++i)
	{
		if (FF.Blocked[i]) { continue; }

		const int32 CurX = i % FFlowField::GridSize;
		const int32 CurY = i / FFlowField::GridSize;
		const float CurHeight = FF.Heights[i];
		uint8 Edges = 0;

		for (int32 d = 0; d < 8; ++d)
		{
			const int32 NX = CurX + OffX[d];
			const int32 NY = CurY + OffY[d];

			if (NX < 0 || NX >= FFlowField::GridSize ||
			    NY < 0 || NY >= FFlowField::GridSize)
			{
				Edges |= (1 << d);
				continue;
			}

			const int32 NIdx = NY * FFlowField::GridSize + NX;

			if (FF.Blocked[NIdx])
			{
				Edges |= (1 << d);
				continue;
			}

			const float HeightDiff = FMath::Abs(FF.Heights[NIdx] - CurHeight);
			const float SlopeAngle = FMath::Atan2(HeightDiff, HorizDist[d]);

			if (SlopeAngle > FFlowField::MaxWalkableAngleRad)
			{	
				Edges |= (1 << d);
			}
		}

		FF.CliffEdges[i] = Edges;
	}
}

/** Pass 3: BFS from GoalIndex → CostField + Directions (순수 수학, Worker-safe) */
void RunBFS(FFlowField& FF, int32 GoalIndex)
{
	// Reset
	FMemory::Memset(FF.CostField.GetData(), 0xFF, FFlowField::TotalCells * sizeof(uint16));
	FMemory::Memzero(FF.Directions.GetData(), FFlowField::TotalCells * sizeof(FVector2f));

	if (GoalIndex == INDEX_NONE || FF.Blocked[GoalIndex]) { return; }

	// BFS 큐
	TArray<int32> Queue;
	Queue.Reserve(FFlowField::TotalCells);

	FF.CostField[GoalIndex] = 0;
	Queue.Add(GoalIndex);
	int32 QueueHead = 0;

	while (QueueHead < Queue.Num())
	{
		const int32 Current = Queue[QueueHead++];
		const uint16 CurrentCost = FF.CostField[Current];
		const int32 CurX = Current % FFlowField::GridSize;
		const int32 CurY = Current / FFlowField::GridSize;
		const uint8 CurEdges = FF.CliffEdges[Current];

		for (int32 d = 0; d < 8; ++d)
		{
			// 절벽 엣지 체크
			if (CurEdges & (1 << d)) { continue; }

			const int32 NX = CurX + OffX[d];
			const int32 NY = CurY + OffY[d];

			if (NX < 0 || NX >= FFlowField::GridSize ||
			    NY < 0 || NY >= FFlowField::GridSize) { continue; }

			const int32 NIdx = NY * FFlowField::GridSize + NX;

			if (FF.Blocked[NIdx]) { continue; }

			const uint16 NewCost = CurrentCost + 1;
			if (FF.CostField[NIdx] <= NewCost) { continue; }

			FF.CostField[NIdx] = NewCost;
			Queue.Add(NIdx);
		}
	}

	// Direction 패스: 각 셀에서 비용이 가장 낮은 이웃 방향
	for (int32 i = 0; i < FFlowField::TotalCells; ++i)
	{
		if (FF.CostField[i] == 0 || FF.CostField[i] == UINT16_MAX) { continue; }

		const int32 CurX = i % FFlowField::GridSize;
		const int32 CurY = i / FFlowField::GridSize;
		const uint8 CurEdges = FF.CliffEdges[i];
		uint16 BestCost = FF.CostField[i];
		int32 BestDX = 0;
		int32 BestDY = 0;

		for (int32 d = 0; d < 8; ++d)
		{
			if (CurEdges & (1 << d)) { continue; }

			const int32 NX = CurX + OffX[d];
			const int32 NY = CurY + OffY[d];

			if (NX < 0 || NX >= FFlowField::GridSize ||
			    NY < 0 || NY >= FFlowField::GridSize) { continue; }

			const int32 NIdx = NY * FFlowField::GridSize + NX;
			if (FF.CostField[NIdx] < BestCost)
			{
				BestCost = FF.CostField[NIdx];
				BestDX = OffX[d];
				BestDY = OffY[d];
			}
		}

		if (BestDX != 0 || BestDY != 0)
		{
			FVector2f Dir(static_cast<float>(BestDX), static_cast<float>(BestDY));
			Dir.Normalize();
			FF.Directions[i] = Dir;
		}
	}
}

} // anonymous namespace

// ── FlowFieldSystem ──

void FlowFieldSystem::BuildTerrainCache(FFlowField& FlowField, UWorld* World, const FVector& MapCenter)
{
	if (!World) { return; }

	// 그리드 원점 설정 (맵 중심 기준)
	FlowField.OriginX = MapCenter.X - FFlowField::HalfExtent;
	FlowField.OriginY = MapCenter.Y - FFlowField::HalfExtent;

	// Pass 1: 라인트레이스 [GameThread]
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;

	const float TraceStartZ = MapCenter.Z + 5000.f;
	const float TraceEndZ   = MapCenter.Z - 5000.f;

	for (int32 i = 0; i < FFlowField::TotalCells; ++i)
	{
		const FVector2f CellCenter = FlowField.IndexToWorld(i);
		const FVector Start(CellCenter.X, CellCenter.Y, TraceStartZ);
		const FVector End(CellCenter.X, CellCenter.Y, TraceEndZ);

		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, QueryParams))
		{
			FlowField.Heights[i] = Hit.ImpactPoint.Z;
		}
		else
		{
			FlowField.Blocked[i] = true;
			FlowField.Heights[i] = FFlowField::InvalidHeight;
		}
	}
}

void FlowFieldSystem::BuildCliffEdgesAndBFS(FFlowField& FlowField, int32 GoalIndex)
{
	BuildCliffEdges(FlowField);
	RunBFS(FlowField, GoalIndex);
	FlowField.bReady = true;
}