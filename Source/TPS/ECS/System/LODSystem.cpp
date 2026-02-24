#include "ECS/System/LODSystem.h"

#include "Async/ParallelFor.h"
#include "ECS/Component/Components.h"

/** ② Write: LOD Level + TickInterval + bShouldTick 갱신 */
void Write(CLOD& OutLOD, ELODLevel NewLevel, int32 NewTickInterval,
           uint8 bNewShouldTick)
{
	OutLOD.Level        = NewLevel;
	OutLOD.TickInterval = NewTickInterval;
	OutLOD.bShouldTick  = bNewShouldTick;
}

/** ③ PushToPrev: CLOD → CLODPrev 복사 */
void PushToPrev(CLODPrev& OutPrev, const CLOD& InCurrent)
{
	OutPrev.Level                = InCurrent.Level;
	OutPrev.FrameOffset          = InCurrent.FrameOffset;
	OutPrev.TickInterval         = InCurrent.TickInterval;
	OutPrev.AccumulatedDeltaTime = InCurrent.AccumulatedDeltaTime;
	OutPrev.bShouldTick          = InCurrent.bShouldTick;
}

void LODSystem::Tick(entt::registry& Registry, const FVector& PlayerPosition,
                     float DeltaTime, uint32 FrameCounter)
{
	auto View = Registry.view<CLOD, CLODPrev, CTransformPrev>();

	// Entity 수집 (GameThread — View 순회는 단일 스레드)
	TArray<entt::entity> Entities;
	Entities.Reserve(static_cast<int32>(View.size_hint()));
	for (auto Entity : View)
	{
		Entities.Add(Entity);
	}

	const int32 Count = Entities.Num();
	if (Count == 0) { return; }

	// ParallelFor — Entity별 독립 처리, 공유 상태 없음
	ParallelFor(Count, [&View, &Entities, &PlayerPosition, DeltaTime, FrameCounter](int32 Index)
	{
		const entt::entity Entity = Entities[Index];

		auto& LOD           = View.get<CLOD>(Entity);
		const auto& LODPrev = View.get<CLODPrev>(Entity);

		// ① 이전 프레임에 틱했으면 AccumDT 리셋
		if (LODPrev.bShouldTick)
		{
			LOD.AccumulatedDeltaTime = 0.f;
		}

		// ② 이번 프레임 DeltaTime 누적
		LOD.AccumulatedDeltaTime += DeltaTime;

		// ③ Read: 거리² 계산
		const FVector CachedPosition = View.get<CTransformPrev>(Entity).Position;
		const float DistSq = (CachedPosition - PlayerPosition).SizeSquared();

		// ④ 히스테리시스 기반 LOD Level 결정
		//    올라갈 때(Near→Mid→Far)와 내려올 때(Far→Mid→Near) 임계값 분리
		const ELODLevel CachedLevel = LODPrev.Level;
		ELODLevel NewLevel = {};
		int32 NewTickInterval = 0;

		switch (CachedLevel)
		{
		case ELODLevel::Near:
			if (DistSq > ECSConstants::LODNearRadiusSq)
			{
				NewLevel        = ELODLevel::Mid;
				NewTickInterval = ECSConstants::LODMidTickInterval;
			}
			else
			{
				NewLevel        = ELODLevel::Near;
				NewTickInterval = ECSConstants::LODNearTickInterval;
			}
			break;

		case ELODLevel::Mid:
			if (DistSq <= ECSConstants::LODMidToNearRadiusSq)
			{
				NewLevel        = ELODLevel::Near;
				NewTickInterval = ECSConstants::LODNearTickInterval;
			}
			else if (DistSq > ECSConstants::LODMidRadiusSq)
			{
				NewLevel        = ELODLevel::Far;
				NewTickInterval = ECSConstants::LODFarTickInterval;
			}
			else
			{
				NewLevel        = ELODLevel::Mid;
				NewTickInterval = ECSConstants::LODMidTickInterval;
			}
			break;

		case ELODLevel::Far:
			if (DistSq <= ECSConstants::LODFarToMidRadiusSq)
			{
				NewLevel        = ELODLevel::Mid;
				NewTickInterval = ECSConstants::LODMidTickInterval;
			}
			else
			{
				NewLevel        = ELODLevel::Far;
				NewTickInterval = ECSConstants::LODFarTickInterval;
			}
			break;
		}

		// ⑤ 이번 프레임 틱 여부 결정
		const uint8 bNewShouldTick =
			((FrameCounter + LOD.FrameOffset) % NewTickInterval == 0) ? 1 : 0;

		// ⑥ Write
		Write(LOD, NewLevel, NewTickInterval, bNewShouldTick);

		// ⑦ PushToPrev
		PushToPrev(View.get<CLODPrev>(Entity), LOD);
	});
}