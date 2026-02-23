#include "ECS/System/SpawnSystem.h"
#include "ECS/Component/Components.h"

entt::entity SpawnSystem::Spawn(entt::registry& Registry,
                                const FVector& Position,
                                float MaxHealth,
                                float MaxSpeed)
{
	auto Entity = Registry.create();

	// ① Current
	Registry.emplace<CEnemyState>(Entity);
	Registry.emplace<CTransform>(Entity, Position, FQuat::Identity);
	Registry.emplace<CHealth>(Entity, MaxHealth, MaxHealth);
	Registry.emplace<CMovement>(Entity, FVector::ZeroVector, MaxSpeed);
	Registry.emplace<CRenderProxy>(Entity);
	Registry.emplace<CAnimation>(Entity);

	// ② Prev (초기값 = Current와 동일)
	Registry.emplace<CEnemyStatePrev>(Entity);
	Registry.emplace<CTransformPrev>(Entity, Position, FQuat::Identity);
	Registry.emplace<CHealthPrev>(Entity, MaxHealth, MaxHealth);
	Registry.emplace<CMovementPrev>(Entity, FVector::ZeroVector, MaxSpeed);
	Registry.emplace<CRenderProxyPrev>(Entity);
	Registry.emplace<CAnimationPrev>(Entity);

	return Entity;
}
