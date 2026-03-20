#include "ECS/System/VisualizationSystem.h"
#include "ECS/Component/Components.h"
#include "Components/InstancedStaticMeshComponent.h"

namespace
{

/** PushToPrev: CVisCache → CVisCachePrev */
void PushToPrev(CVisCachePrev& OutPrev, const CVisCache& InCurrent)
{
	OutPrev.Position  = InCurrent.Position;
	OutPrev.Rotation  = InCurrent.Rotation;
	OutPrev.AnimIndex = InCurrent.AnimIndex;
	OutPrev.AnimTime  = InCurrent.AnimTime;
}

} // anonymous namespace

void VisualizationSystem::Tick(entt::registry& Registry,
                               UInstancedStaticMeshComponent* HISM,
                               uint8 LODLevel)
{
	if (!ensure(HISM)) { return; }

	auto View = Registry.view<CRenderProxyPrev, CTransformPrev, CAnimationPrev,
	                          CLODPrev, CVisCache, CVisCachePrev, CMeshOffset>();

	bool bDirty = false;

	for (auto Entity : View)
	{
		const auto& ProxyPrev = View.get<CRenderProxyPrev>(Entity);

		// LOD 필터 — 이 HISM에 속한 Entity만 처리
		if (ProxyPrev.LODLevel != LODLevel) { continue; }

		// ① Read — Prev → 지역변수
		const int32 CachedIndex = ProxyPrev.InstanceIndex;
		if (CachedIndex == INDEX_NONE) { continue; }

		const bool bCachedShouldTick = View.get<CLODPrev>(Entity).bShouldTick;
		if (!bCachedShouldTick) { continue; }

		const FVector CachedPosition = View.get<CTransformPrev>(Entity).Position;
		const FQuat CachedRotation   = View.get<CTransformPrev>(Entity).Rotation;
		const float CachedAnimIndex  = View.get<CAnimationPrev>(Entity).AnimIndex;
		const float CachedAnimTime   = View.get<CAnimationPrev>(Entity).AnimTime;

		// 변경 감지 — CVisCachePrev (HISM에 마지막 기록한 값)와 비교
		const CVisCachePrev& CachedVC = View.get<CVisCachePrev>(Entity);

		const bool bTransformDirty = (CachedPosition != CachedVC.Position)
		                          || (CachedRotation != CachedVC.Rotation);
		const bool bAnimDirty      = (CachedAnimIndex != CachedVC.AnimIndex)
		                          || (CachedAnimTime != CachedVC.AnimTime);

		if (!bTransformDirty && !bAnimDirty) { continue; }

		// ② Write — 변경된 부분만 HISM 갱신
		if (bTransformDirty)
		{
			const FQuat MeshRot = CachedRotation * View.get<CMeshOffset>(Entity).RotationOffset;
			const FTransform InstanceTransform(MeshRot, CachedPosition);
			HISM->UpdateInstanceTransform(CachedIndex, InstanceTransform,
			                              false, false);
		}

		if (bAnimDirty)
		{
			HISM->SetCustomDataValue(CachedIndex, 0, CachedAnimIndex);
			HISM->SetCustomDataValue(CachedIndex, 1, CachedAnimTime);
		}

		bDirty = true;

		// ③ Write — VisCache 갱신
		CVisCache& OutCache = View.get<CVisCache>(Entity);
		OutCache.Position   = CachedPosition;
		OutCache.Rotation   = CachedRotation;
		OutCache.AnimIndex  = CachedAnimIndex;
		OutCache.AnimTime   = CachedAnimTime;

		// ④ PushToPrev
		PushToPrev(View.get<CVisCachePrev>(Entity), OutCache);
	}

	// 실제 갱신이 있었을 때만 리빌드
	if (bDirty)
	{
		HISM->MarkRenderStateDirty();
	}
}
