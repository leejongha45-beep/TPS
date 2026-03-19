#include "ECS/System/LODSystem.h"

#include "Async/ParallelFor.h"
#include "ECS/Component/Components.h"

namespace
{

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

} // anonymous namespace

void LODSystem::Tick(entt::registry& Registry, const FVector& PlayerPosition,
                     float DeltaTime, uint32 FrameCounter)
{
	auto View = Registry.view<CLOD, CLODPrev, CTransformPrev>();

	// Entity 수집 (GameThread — View 순회는 단일 스레드)
	TArray<entt::entity, TInlineAllocator<3000>> Entities;
	Entities.Reserve(View.size_hint());
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
		// NOTE: AccumDT는 Current에 직접 읽기/쓰기 — 누적 특성상 Read→Prev 패턴 적용 불가
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

void LODSystem::TransitionInstances(entt::registry& Registry,
                                    UInstancedStaticMeshComponent* const* HISMRefs,
                                    TArray<entt::entity>* InstanceToEntityPerLOD)
{
	auto View = Registry.view<CRenderProxy, CLOD, CLODPrev,
	                          CTransformPrev, CAnimationPrev>();

	for (auto Entity : View)
	{
		auto& Proxy = View.get<CRenderProxy>(Entity);
		const uint8 NewLOD = static_cast<uint8>(View.get<CLOD>(Entity).Level);
		const uint8 OldLOD = Proxy.LODLevel;

		if (NewLOD == OldLOD || Proxy.InstanceIndex == INDEX_NONE) { continue; }

		auto* pOldISM = HISMRefs[OldLOD];
		auto* pNewISM = HISMRefs[NewLOD];
		if (!pOldISM || !pNewISM) { continue; }

		// ① 이전 ISM에서 제거 (swap-back 보정)
		const int32 OldIndex = Proxy.InstanceIndex;
		const int32 OldLast = pOldISM->GetInstanceCount() - 1;

		pOldISM->RemoveInstance(OldIndex);

		if (OldIndex != OldLast)
		{
			entt::entity SwappedEntity = InstanceToEntityPerLOD[OldLOD][OldLast];
			Registry.get<CRenderProxy>(SwappedEntity).InstanceIndex = OldIndex;
			InstanceToEntityPerLOD[OldLOD][OldIndex] = SwappedEntity;
		}
		InstanceToEntityPerLOD[OldLOD].Pop();

		// ② 새 ISM에 추가
		const FVector& Pos = View.get<CTransformPrev>(Entity).Position;
		const FTransform InstanceTransform(FQuat::Identity, Pos);
		const int32 NewIndex = pNewISM->AddInstance(InstanceTransform, true);

		// 커스텀 데이터 (VAT) 복사
		const float AnimIdx = View.get<CAnimationPrev>(Entity).AnimIndex;
		const float AnimTime = View.get<CAnimationPrev>(Entity).AnimTime;
		pNewISM->SetCustomDataValue(NewIndex, 0, AnimIdx);
		pNewISM->SetCustomDataValue(NewIndex, 1, AnimTime);

		// ③ 프록시 갱신
		Proxy.InstanceIndex = NewIndex;
		Proxy.LODLevel = NewLOD;
		InstanceToEntityPerLOD[NewLOD].Add(Entity);
	}
}