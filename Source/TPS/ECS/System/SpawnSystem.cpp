#include "ECS/System/SpawnSystem.h"
#include "ECS/Component/Components.h"

entt::entity SpawnSystem::Spawn(entt::registry& Registry,
                                const FVector& Position,
                                float MaxHealth,
                                float MaxSpeed)
{
	auto Entity = Registry.create();
	const float InitAttackTimer = FMath::RandRange(0.f, ECSConstants::AttackCooldown);

	const uint8 FrameOffset = static_cast<uint8>(
		static_cast<uint32>(entt::to_integral(Entity)) % ECSConstants::LODFarTickInterval);

	// ① Current
	Registry.emplace<CEnemyState>(Entity);
	Registry.emplace<CTransform>(Entity, Position, FQuat::Identity);
	Registry.emplace<CHealth>(Entity, MaxHealth, MaxHealth);
	Registry.emplace<CMovement>(Entity, FVector::ZeroVector, MaxSpeed);
	Registry.emplace<CRenderProxy>(Entity);
	Registry.emplace<CAnimation>(Entity);
	Registry.emplace<CAttack>(Entity, ECSConstants::AttackDamage,
	                           ECSConstants::AttackCooldown, InitAttackTimer);
	Registry.emplace<CLOD>(Entity, ELODLevel::Near, FrameOffset,
	                        ECSConstants::LODNearTickInterval, 0.f, true);
	Registry.emplace<CVisCache>(Entity);

	// ② Prev (초기값 = Current와 동일)
	Registry.emplace<CEnemyStatePrev>(Entity);
	Registry.emplace<CTransformPrev>(Entity, Position, FQuat::Identity);
	Registry.emplace<CHealthPrev>(Entity, MaxHealth, MaxHealth);
	Registry.emplace<CMovementPrev>(Entity, FVector::ZeroVector, MaxSpeed);
	Registry.emplace<CRenderProxyPrev>(Entity);
	Registry.emplace<CAnimationPrev>(Entity);
	Registry.emplace<CAttackPrev>(Entity, ECSConstants::AttackDamage,
	                               ECSConstants::AttackCooldown, InitAttackTimer);
	Registry.emplace<CLODPrev>(Entity, ELODLevel::Near, FrameOffset,
	                            ECSConstants::LODNearTickInterval, 0.f, true);
	Registry.emplace<CVisCachePrev>(Entity);

	return Entity;
}