#include "ECS/System/VisualizationSystem.h"
#include "ECS/Component/Components.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

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
                               UHierarchicalInstancedStaticMeshComponent* HISM)
{
	if (!ensure(HISM)) { return; }

	auto View = Registry.view<CRenderProxyPrev, CTransformPrev, CAnimationPrev,
	                          CLODPrev, CVisCache, CVisCachePrev>();

	for (auto Entity : View)
	{
		// ① Read: Prev → Cached 지역변수
		const int32 CachedIndex = View.get<CRenderProxyPrev>(Entity).InstanceIndex;
		if (CachedIndex == INDEX_NONE) { continue; }

		// LOD 스킵 — Prev 값 불변이므로 HISM 데이터 유지
		if (!View.get<CLODPrev>(Entity).bShouldTick) { continue; }

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

		// ② Write: 변경된 부분만 HISM 갱신
		if (bTransformDirty)
		{
			const FTransform InstanceTransform(CachedRotation, CachedPosition);
			HISM->UpdateInstanceTransform(CachedIndex, InstanceTransform,
			                              false, false);
		}

		if (bAnimDirty)
		{
			HISM->SetCustomDataValue(CachedIndex, 0, CachedAnimIndex);
			HISM->SetCustomDataValue(CachedIndex, 1, CachedAnimTime);
		}

		// ③ CVisCache 갱신 + PushToPrev
		CVisCache& OutCache = View.get<CVisCache>(Entity);
		OutCache.Position   = CachedPosition;
		OutCache.Rotation   = CachedRotation;
		OutCache.AnimIndex  = CachedAnimIndex;
		OutCache.AnimTime   = CachedAnimTime;

		PushToPrev(View.get<CVisCachePrev>(Entity), OutCache);
	}

	// 전체 갱신 완료 후 한 번만 리빌드
	HISM->MarkRenderStateDirty();
}
