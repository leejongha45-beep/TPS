#pragma once

#include <entt/entt.hpp>

class IDamageable;

/**
 * 공격 시스템 — 쿨다운 기반 적 → 플레이어 데미지 수행
 *
 * [GameThread] Phase 3.1 — AISystem 이후, SeparationSystem 이전
 * - AttackReady/Attacking 상태 Entity의 쿨다운 틱
 * - 쿨다운 만료 시 데미지 집계 → 프레임당 1회 IDamageable::ReceiveDamage 호출
 *
 * Read:  CAttackPrev, CEnemyStatePrev
 * Write: CAttack, CEnemyState → PushToPrev → CAttackPrev, CEnemyStatePrev
 */
namespace AttackSystem
{
	void Tick(entt::registry& Registry, float DeltaTime, IDamageable* PlayerDamageable);
};
