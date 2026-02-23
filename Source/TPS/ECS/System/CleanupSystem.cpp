#include "ECS/System/CleanupSystem.h"
#include "ECS/Component/Components.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include <algorithm>
#include <span>

namespace
{
	struct FDeadEntry
	{
		entt::entity Entity;
		int32 InstanceIndex;
	};
}

/** ② Write: HISM RemoveInstance (내림차순) + O(1) swap 보정 + Entity 파괴 */
void Write(entt::registry& Registry, UHierarchicalInstancedStaticMeshComponent* HISM,
           TArray<entt::entity>& InstanceToEntity, std::span<FDeadEntry> DeadEntries)
{
	// 내림차순 정렬 — 높은 InstanceIndex부터 제거해야 swap이 미처리 Dead에 영향 없음
	std::sort(DeadEntries.begin(), DeadEntries.end(), [](const FDeadEntry& A, const FDeadEntry& B)
	{
		return A.InstanceIndex > B.InstanceIndex;
	});

	for (auto& Entry : DeadEntries)
	{
		// ②-1. HISM RemoveInstance
		if (Entry.InstanceIndex != INDEX_NONE)
		{
			const int32 LastIndex = HISM->GetInstanceCount() - 1;

			HISM->RemoveInstance(Entry.InstanceIndex);

			// ②-2. O(1) swap 보정 — 룩업 테이블로 즉시 Entity 특정
			// Current만 쓰기 — 다음 프레임 PushToPrev_RenderProxy에서 Prev 반영
			if (Entry.InstanceIndex != LastIndex)
			{
				entt::entity SwappedEntity = InstanceToEntity[LastIndex];

				Registry.get<CRenderProxy>(SwappedEntity).InstanceIndex = Entry.InstanceIndex;
				InstanceToEntity[Entry.InstanceIndex] = SwappedEntity;
			}

			// ②-3. 테이블 축소 — 마지막 슬롯 제거
			InstanceToEntity.Pop();
		}

		// ②-4. Entity 파괴
		Registry.destroy(Entry.Entity);
	}
}

void CleanupSystem::Tick(entt::registry& Registry, UHierarchicalInstancedStaticMeshComponent* HISM,
                         TArray<entt::entity>& InstanceToEntity)
{
	if (!ensure(HISM)) { return; }

	auto View = Registry.view<CEnemyStatePrev, CRenderProxyPrev>();

	FDeadEntry DeadEntries[3000];
	int32 Count = 0;

	// ① Read: Dead Entity + InstanceIndex 수집
	for (auto Entity : View)
	{
		const EEnemyState CachedState = View.get<CEnemyStatePrev>(Entity).State;
		if (CachedState != EEnemyState::Dead) { continue; }

		const int32 CachedIndex = View.get<CRenderProxyPrev>(Entity).InstanceIndex;
		DeadEntries[Count++] = { Entity, CachedIndex };
		if (Count >= 3000) { break; }
	}

	if (Count == 0) { return; }

	// ② Write
	Write(Registry, HISM, InstanceToEntity, std::span(DeadEntries, Count));
}