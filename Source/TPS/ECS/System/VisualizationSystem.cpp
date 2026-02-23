#include "ECS/System/VisualizationSystem.h"
#include "ECS/Component/Components.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

void VisualizationSystem::Tick(entt::registry& Registry, UHierarchicalInstancedStaticMeshComponent* HISM)
{
	if (!ensure(HISM)) { return; }

	auto View = Registry.view<CRenderProxyPrev, CTransformPrev, CAnimationPrev>();

	for (auto Entity : View)
	{
		// ① Read: Prev → Cached 지역변수
		const int32 CachedIndex       = View.get<CRenderProxyPrev>(Entity).InstanceIndex;
		const FVector CachedPosition  = View.get<CTransformPrev>(Entity).Position;
		const FQuat CachedRotation    = View.get<CTransformPrev>(Entity).Rotation;
		const float CachedAnimIndex   = View.get<CAnimationPrev>(Entity).AnimIndex;
		const float CachedAnimTime    = View.get<CAnimationPrev>(Entity).AnimTime;

		if (CachedIndex == INDEX_NONE) { continue; }

		// ② HISM 갱신 — Transform
		const FTransform InstanceTransform(CachedRotation, CachedPosition);
		HISM->UpdateInstanceTransform(CachedIndex, InstanceTransform, false, false);

		// ③ HISM 갱신 — VAT CustomData
		HISM->SetCustomDataValue(CachedIndex, 0, CachedAnimIndex);
		HISM->SetCustomDataValue(CachedIndex, 1, CachedAnimTime);
	}

	// 전체 갱신 완료 후 한 번만 리빌드
	HISM->MarkRenderStateDirty();
}
