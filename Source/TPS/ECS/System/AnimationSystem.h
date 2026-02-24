#pragma once

#include <entt/entt.hpp>

/**
 * 애니메이션 시스템 — VAT AnimTime 갱신
 *
 * [WorkerThread] Phase 5 — MovementSystem과 TaskGraph 병렬 실행
 * - 내부 ParallelFor로 Entity별 병렬 처리
 * - Read:  CAnimationPrev (AnimIndex, AnimTime, PlayRate), CEnemyStatePrev,
 *          CLODPrev (bShouldTick, AccumulatedDeltaTime)
 * - Write: CAnimation (AnimIndex, AnimTime)
 * - PushToPrev: CAnimation → CAnimationPrev
 *
 * 스레드 안전성:
 * - MovementSystem과 Write 대상 완전 분리 (CAnimation vs CTransform)
 * - CEnemyStatePrev, CLODPrev는 양쪽 모두 Read-Only → 공유 읽기 안전
 * - LOD 스킵: bShouldTick=false → Write/PushToPrev 건너뜀 → AnimTime 유지
 *   틱 시 AccumulatedDeltaTime으로 스킵 프레임 시간 보상
 */
namespace AnimationSystem
{
	void Tick(entt::registry& Registry, float DeltaTime);
};
