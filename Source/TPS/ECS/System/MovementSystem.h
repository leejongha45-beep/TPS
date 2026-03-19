#pragma once

#include "Math/Vector.h"
#include "ThirdParty/EnTT/include/entt/entity/registry.hpp"

struct FFlowField;

/**
 * 이동 시스템 — CTransform 갱신 + Z값 지형 보정
 *
 * UpdateChaseTargets: [GameThread] Phase 1.1
 * - Chase 엔티티 NavMesh 경로 쿼리 + CNavTarget PushToPrev
 * - Read:  CAIModePrev, CTransformPrev
 * - Write: CNavTarget
 * - PushToPrev: CNavTarget → CNavTargetPrev
 *
 * Tick: [WorkerThread] Phase 6
 * - 내부 ParallelFor로 Entity별 병렬 처리
 * - Read:  CMovementPrev.Velocity, CEnemyStatePrev, CLODPrev, FFlowField.Heights
 * - Write: CTransform.Position
 * - PushToPrev: CTransform → CTransformPrev
 */
namespace MovementSystem
{
	void UpdateChaseTargets(entt::registry& Registry, class UWorld* InWorld,
	                        const FVector& PlayerPosition, int32 FrameCounter);
	void Tick(entt::registry& Registry, float DeltaTime, const FFlowField& FlowField);
};