#include "ECS/System/SpawnSystem.h"
#include "ECS/Component/Components.h"
#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

entt::entity SpawnSystem::Spawn(entt::registry& Registry,
                                const FEnemySpawnParams& Params)
{
	auto Entity = Registry.create();
	const float InitAttackTimer = FMath::RandRange(0.f, Params.AttackCooldown);

	const uint8 FrameOffset = static_cast<uint8>(
		static_cast<uint32>(entt::to_integral(Entity)) % ECSConstants::LODFarTickInterval);

	// ① Current
	Registry.emplace<CEnemyState>(Entity);
	Registry.emplace<CTransform>(Entity, Params.Position, FQuat::Identity);
	Registry.emplace<CHealth>(Entity, Params.MaxHealth, Params.MaxHealth);
	Registry.emplace<CMovement>(Entity, FVector::ZeroVector, Params.MaxSpeed);
	Registry.emplace<CRenderProxy>(Entity);
	Registry.emplace<CAnimation>(Entity);
	Registry.emplace<CAttack>(Entity, Params.AttackDamage,
	                           Params.AttackCooldown, InitAttackTimer);
	Registry.emplace<CLOD>(Entity, ELODLevel::Near, FrameOffset,
	                        ECSConstants::LODNearTickInterval, 0.f, true);
	Registry.emplace<CVisCache>(Entity);
	Registry.emplace<CAIMode>(Entity);
	Registry.emplace<CNavTarget>(Entity);

	// ② Prev (초기값 = Current와 동일)
	Registry.emplace<CEnemyStatePrev>(Entity);
	Registry.emplace<CTransformPrev>(Entity, Params.Position, FQuat::Identity);
	Registry.emplace<CHealthPrev>(Entity, Params.MaxHealth, Params.MaxHealth);
	Registry.emplace<CMovementPrev>(Entity, FVector::ZeroVector, Params.MaxSpeed);
	Registry.emplace<CRenderProxyPrev>(Entity);
	Registry.emplace<CAnimationPrev>(Entity);
	Registry.emplace<CAttackPrev>(Entity, Params.AttackDamage,
	                               Params.AttackCooldown, InitAttackTimer);
	Registry.emplace<CLODPrev>(Entity, ELODLevel::Near, FrameOffset,
	                            ECSConstants::LODNearTickInterval, 0.f, true);
	Registry.emplace<CVisCachePrev>(Entity);
	Registry.emplace<CAIModePrev>(Entity);
	Registry.emplace<CNavTargetPrev>(Entity);

	return Entity;
}