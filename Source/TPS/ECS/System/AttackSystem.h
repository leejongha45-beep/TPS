#pragma once

#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

class IDamageable;

/**
 * 공격 시스템 — 3단계 상태 머신 기반 적 → 플레이어 데미지 수행
 *
 * [GameThread] Phase 3.1 — AISystem 이후, SeparationSystem 이전
 * - AttackCooldown → 쿨다운 틱 → AttackReady → AnimTime 기반 → Attacking + 데미지
 * - 프레임당 1회 IDamageable::ReceiveDamage 집계 호출
 *
 * Read:  CAttackPrev, CEnemyStatePrev, CAnimationPrev
 * Write: CAttack, CEnemyState → PushToPrev → CAttackPrev, CEnemyStatePrev
 */
namespace AttackSystem
{
	void Tick(entt::registry& Registry, float DeltaTime, IDamageable* PlayerDamageable);
};