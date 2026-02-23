#pragma once

#include <entt/entt.hpp>

/**
 * 애니메이션 시스템 — VAT AnimTime 갱신
 *
 * [WorkerThread] Phase 5 — MovementSystem과 TaskGraph 병렬 실행
 * - 내부 ParallelFor로 Entity별 병렬 처리
 * - Read:  CAnimationPrev (AnimIndex, AnimTime, PlayRate), CEnemyStatePrev
 * - Write: CAnimation (AnimIndex, AnimTime)
 * - PushToPrev: CAnimation → CAnimationPrev
 *
 * 스레드 안전성:
 * - MovementSystem과 Write 대상 완전 분리 (CAnimation vs CTransform)
 * - CEnemyStatePrev는 양쪽 모두 Read-Only → 공유 읽기 안전
 */
namespace AnimationSystem
{
	void Tick(entt::registry& Registry, float DeltaTime);
};
