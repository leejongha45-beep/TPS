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

		// ① Read — Prev → 지역변수
		const FVector CachedPosition = View.get<CTransformPrev>(Entity).Position;
		const ELODLevel CachedLevel = View.get<CLODPrev>(Entity).Level;
		const bool bPrevTicked = View.get<CLODPrev>(Entity).bShouldTick;
		const float CachedAccumDT = View.get<CLODPrev>(Entity).AccumulatedDeltaTime;
		const uint32 CachedFrameOffset = View.get<CLODPrev>(Entity).FrameOffset;

		// ② 계산 — 지역변수만 사용
		const float NewAccumDT = bPrevTicked ? DeltaTime : (CachedAccumDT + DeltaTime);
		const float DistSq = (CachedPosition - PlayerPosition).SizeSquared();

		// 히스테리시스 기반 LOD Level 결정
		//   올라갈 때(Near→Mid→Far)와 내려올 때(Far→Mid→Near) 임계값 분리
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

		// 이번 프레임 틱 여부 결정
		const uint8 bNewShouldTick =
			((FrameCounter + CachedFrameOffset) % NewTickInterval == 0) ? 1 : 0;

		// ③ Write — 계산 결과 → Current에 쓰기
		auto& LOD = View.get<CLOD>(Entity);
		LOD.AccumulatedDeltaTime = NewAccumDT;
		Write(LOD, NewLevel, NewTickInterval, bNewShouldTick);

		// ④ PushToPrev — Current → Prev
		PushToPrev(View.get<CLODPrev>(Entity), LOD);
	});
}

void LODSystem::TransitionInstances(entt::registry& Registry,
                                    UInstancedStaticMeshComponent* const* HISMRefs,
                                    TArray<entt::entity>* InstanceToEntityPerLOD)
{
	auto View = Registry.view<CRenderProxy, CRenderProxyPrev, CLODPrev,
	                          CTransformPrev, CAnimationPrev>();

	for (auto Entity : View)
	{
		// ① Read
		// NOTE: InstanceIndex/LODLevel은 Current에서 읽음 — swap-back이 Current를 갱신하므로
		//       Prev에서 읽으면 swap된 엔티티가 stale 인덱스로 접근하여 크래시
		const uint8 CachedNewLOD = static_cast<uint8>(View.get<CLODPrev>(Entity).Level);
		const uint8 CachedOldLOD = View.get<CRenderProxy>(Entity).LODLevel;
		const int32 CachedInstanceIndex = View.get<CRenderProxy>(Entity).InstanceIndex;

		if (CachedNewLOD == CachedOldLOD || CachedInstanceIndex == INDEX_NONE) { continue; }

		auto* pOldISM = HISMRefs[CachedOldLOD];
		auto* pNewISM = HISMRefs[CachedNewLOD];
		if (!pOldISM || !pNewISM) { continue; }

		const FVector CachedPosition = View.get<CTransformPrev>(Entity).Position;
		const float CachedAnimIdx = View.get<CAnimationPrev>(Entity).AnimIndex;
		const float CachedAnimTime = View.get<CAnimationPrev>(Entity).AnimTime;

		// ② Write — ISM 인스턴스 이동 + 프록시 갱신

		// 기존 인스턴스 제거 (Far에서 오는 경우 INDEX_NONE이므로 스킵)
		if (CachedInstanceIndex != INDEX_NONE)
		{
			const int32 OldLast = pOldISM->GetInstanceCount() - 1;
			pOldISM->RemoveInstance(CachedInstanceIndex);

			if (CachedInstanceIndex != OldLast)
			{
				entt::entity SwappedEntity = InstanceToEntityPerLOD[CachedOldLOD][OldLast];
				Registry.get<CRenderProxy>(SwappedEntity).InstanceIndex = CachedInstanceIndex;
				InstanceToEntityPerLOD[CachedOldLOD][CachedInstanceIndex] = SwappedEntity;
			}
			InstanceToEntityPerLOD[CachedOldLOD].Pop();
		}

		int32 NewIndex = INDEX_NONE;

		// Far LOD → 렌더링 안 함 (인스턴스 생성 X)
		if (CachedNewLOD != static_cast<uint8>(ELODLevel::Far))
		{
			const FTransform InstanceTransform(FQuat::Identity, CachedPosition);
			NewIndex = pNewISM->AddInstance(InstanceTransform, true);
			pNewISM->SetCustomDataValue(NewIndex, 0, CachedAnimIdx);
			pNewISM->SetCustomDataValue(NewIndex, 1, CachedAnimTime);
			InstanceToEntityPerLOD[CachedNewLOD].Add(Entity);
		}

		auto& Proxy = View.get<CRenderProxy>(Entity);
		Proxy.InstanceIndex = NewIndex;
		Proxy.LODLevel = CachedNewLOD;

		// ③ PushToPrev
		auto& ProxyPrev = View.get<CRenderProxyPrev>(Entity);
		ProxyPrev.InstanceIndex = NewIndex;
		ProxyPrev.LODLevel = CachedNewLOD;
	}
}