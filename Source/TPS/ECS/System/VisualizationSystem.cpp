#include "ECS/System/VisualizationSystem.h"
#include "ECS/Component/Components.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

/** ② Write: Cached 값 → HISM 인스턴스 갱신 (ECS 외부 출력, PushToPrev 불필요) */
void Write(UHierarchicalInstancedStaticMeshComponent* HISM, int32 CachedIndex,
           const FVector& CachedPosition, const FQuat& CachedRotation,
           float CachedAnimIndex, float CachedAnimTime)
{
	const FTransform InstanceTransform(CachedRotation, CachedPosition);
	HISM->UpdateInstanceTransform(CachedIndex, InstanceTransform, false, false);

	HISM->SetCustomDataValue(CachedIndex, 0, CachedAnimIndex);
	HISM->SetCustomDataValue(CachedIndex, 1, CachedAnimTime);
}

void VisualizationSystem::Tick(entt::registry& Registry, UHierarchicalInstancedStaticMeshComponent* HISM)
{
	if (!ensure(HISM)) { return; }

	auto View = Registry.view<CRenderProxyPrev, CTransformPrev, CAnimationPrev>();

	for (auto Entity : View)
	{
		// ① Read: Prev → Cached 지역변수
		const int32 CachedIndex       = View.get<CRenderProxyPrev>(Entity).InstanceIndex;
		if (CachedIndex == INDEX_NONE) { continue; }

		const FVector CachedPosition  = View.get<CTransformPrev>(Entity).Position;
		const FQuat CachedRotation    = View.get<CTransformPrev>(Entity).Rotation;
		const float CachedAnimIndex   = View.get<CAnimationPrev>(Entity).AnimIndex;
		const float CachedAnimTime    = View.get<CAnimationPrev>(Entity).AnimTime;

		// ② Write (ECS 외부 출력 — PushToPrev 불필요)
		Write(HISM, CachedIndex, CachedPosition, CachedRotation, CachedAnimIndex, CachedAnimTime);
	}

	// 전체 갱신 완료 후 한 번만 리빌드
	HISM->MarkRenderStateDirty();
}
