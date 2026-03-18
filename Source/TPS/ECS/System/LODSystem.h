#pragma once

#include "Math/Vector.h"
#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

/**
 * LOD 시스템 — 거리 기반 틱 빈도 제어
 *
 * [GameThread→Worker] Phase 1.5 — UObject 캐시 이후, DamageSystem 이전
 * - Read:  CTransformPrev.Position, CLODPrev (Level, bShouldTick)
 * - Write: CLOD (Level, TickInterval, AccumulatedDeltaTime, bShouldTick)
 * - PushToPrev: CLOD → CLODPrev
 *
 * 히스테리시스 적용:
 *   Near ──30000u──→ Mid ──50000u──→ Far    (올라갈 때)
 *   Near ←──28000u── Mid ←──48000u── Far    (내려올 때)
 *
 * AccumulatedDeltaTime 패턴:
 *   스킵 프레임 동안 DeltaTime 누적 → 틱 프레임에 누적값으로 처리
 *   AI/Movement/Animation/Visualization 4개 시스템이 bShouldTick + AccumDT 사용
 */
namespace LODSystem
{
	void Tick(entt::registry& Registry, const FVector& PlayerPosition,
	          float DeltaTime, uint32 FrameCounter);
};