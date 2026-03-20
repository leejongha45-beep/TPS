#include "ECS/System/CleanupSystem.h"
#include "ECS/Component/Components.h"
#include "Components/InstancedStaticMeshComponent.h"
#include <algorithm>
#include <span>

namespace
{

struct FDeadEntry
{
	entt::entity Entity;
	int32 InstanceIndex;
};

/** ② Write: HISM RemoveInstance (내림차순) + O(1) swap 보정 + Entity 파괴 */
void Write(entt::registry& Registry, UInstancedStaticMeshComponent* HISM,
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
		if (Entry.InstanceIndex != INDEX_NONE && Entry.InstanceIndex < HISM->GetInstanceCount())
		{
			const int32 LastIndex = HISM->GetInstanceCount() - 1;

			HISM->RemoveInstance(Entry.InstanceIndex);

			// ②-2. O(1) swap 보정 — 룩업 테이블로 즉시 Entity 특정
			if (Entry.InstanceIndex != LastIndex && LastIndex < InstanceToEntity.Num())
			{
				entt::entity SwappedEntity = InstanceToEntity[LastIndex];

				if (Registry.valid(SwappedEntity) && Registry.all_of<CRenderProxy>(SwappedEntity))
				{
					Registry.get<CRenderProxy>(SwappedEntity).InstanceIndex = Entry.InstanceIndex;
				}
				InstanceToEntity[Entry.InstanceIndex] = SwappedEntity;
			}

			// ②-3. 테이블 축소 — 마지막 슬롯 제거
			InstanceToEntity.Pop();
		}

		// ②-4. Entity 파괴
		Registry.destroy(Entry.Entity);
	}
}

} // anonymous namespace

int32 CleanupSystem::Tick(entt::registry& Registry, UInstancedStaticMeshComponent* HISM,
                          TArray<entt::entity>& InstanceToEntity, uint8 LODLevel)
{
	if (!ensure(HISM)) { return 0; }

	auto View = Registry.view<CEnemyStatePrev, CRenderProxyPrev>();

	FDeadEntry DeadEntries[3000];
	int32 Count = 0;

	// ① Read: Dead Entity + InstanceIndex 수집 (해당 LOD만)
	for (auto Entity : View)
	{
		const auto& ProxyPrev = View.get<CRenderProxyPrev>(Entity);
		if (ProxyPrev.LODLevel != LODLevel) { continue; }

		const EEnemyState CachedState = View.get<CEnemyStatePrev>(Entity).State;
		if (CachedState != EEnemyState::Dead) { continue; }

		DeadEntries[Count++] = { Entity, ProxyPrev.InstanceIndex };
		if (Count >= 3000) { break; }
	}

	if (Count == 0) { return 0; }

	// ② Write
	Write(Registry, HISM, InstanceToEntity, std::span(DeadEntries, Count));

	return Count;
}
