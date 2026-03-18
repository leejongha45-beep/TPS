#pragma once

#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

/**
 * 이동 시스템 — CTransform 갱신
 *
 * [WorkerThread] Phase 6 — AnimationSystem과 TaskGraph 병렬 실행
 * - 내부 ParallelFor로 Entity별 병렬 처리
 * - Read:  CMovementPrev.Velocity, CEnemyStatePrev (Moving 필터), CLODPrev (bShouldTick, AccumulatedDeltaTime)
 * - Write: CTransform.Position
 * - PushToPrev: CTransform → CTransformPrev
 *
 * 스레드 안전성:
 * - AnimationSystem과 Write 대상 완전 분리 (CTransform vs CAnimation)
 * - CEnemyStatePrev, CLODPrev는 양쪽 모두 Read-Only → 공유 읽기 안전
 * - LOD 스킵: bShouldTick=false → Write/PushToPrev 건너뜀 → Position 유지
 *   틱 시 AccumulatedDeltaTime으로 스킵 프레임 시간 보상
 */
namespace MovementSystem
{
	void Tick(entt::registry& Registry, float DeltaTime);
};