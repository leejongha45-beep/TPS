#pragma once

#include "Math/Vector.h"
#include "Math/Quat.h"

/**
 * ECS 컴포넌트 — 순수 C++ struct (USTRUCT/UPROPERTY 금지)
 * - EnTT registry에 emplace되는 데이터 단위
 * - UE 타입은 FVector, FQuat 등 값 타입만 허용
 * - 모든 컴포넌트는 Current/Prev 쌍으로 운용 (더블 버퍼링)
 */

// ── 상태 열거형 ──

enum class EEnemyState : uint8
{
	Idle,
	Moving,
	AttackReady,
	Attacking,
	Dying,     // 사망 애니메이션 재생 중
	Dead       // 애니메이션 완료 → Cleanup 대상
};

/** ECS 상수 — 시스템 간 공유 */
namespace ECSConstants
{
	constexpr float DeathAnimDuration = 2.f;

	// ── Separation (겹침 방지) ──
	constexpr float SeparationRadius          = 150.f;
	constexpr float SeparationRadiusSq        = SeparationRadius * SeparationRadius;
	constexpr float SeparationWeight          = 0.6f;
	constexpr float MaxSeparationForce        = 200.f;
	constexpr float SeparationCullingRadius   = 3000.f;
	constexpr float SeparationCullingRadiusSq = SeparationCullingRadius * SeparationCullingRadius;
}

// ── Current (쓰기용) ──

/** 적 상태 — AISystem만 Write */
struct CEnemyState
{
	EEnemyState State = EEnemyState::Idle;
};

/** 월드 트랜스폼 */
struct CTransform
{
	FVector Position = FVector::ZeroVector;
	FQuat Rotation = FQuat::Identity;
};

/** 체력 */
struct CHealth
{
	float Current = 0.f;
	float Max = 0.f;
};

/** 이동 */
struct CMovement
{
	FVector Velocity = FVector::ZeroVector;
	float MaxSpeed = 0.f;
};

/** 렌더 프록시 — HISM 인스턴스 인덱스 */
struct CRenderProxy
{
	int32 InstanceIndex = INDEX_NONE;
};

/** 애니메이션 — VAT 재생 상태 */
struct CAnimation
{
	float AnimIndex = 0.f;
	float AnimTime  = 0.f;
	float PlayRate  = 1.f;
};

// ── Prev (읽기용) ──

/** 적 상태 — 이전 프레임 */
struct CEnemyStatePrev
{
	EEnemyState State = EEnemyState::Idle;
};

/** 월드 트랜스폼 — 이전 프레임 */
struct CTransformPrev
{
	FVector Position = FVector::ZeroVector;
	FQuat Rotation = FQuat::Identity;
};

/** 체력 — 이전 프레임 */
struct CHealthPrev
{
	float Current = 0.f;
	float Max = 0.f;
};

/** 이동 — 이전 프레임 */
struct CMovementPrev
{
	FVector Velocity = FVector::ZeroVector;
	float MaxSpeed = 0.f;
};

/** 렌더 프록시 — 이전 프레임 */
struct CRenderProxyPrev
{
	int32 InstanceIndex = INDEX_NONE;
};

/** 애니메이션 — 이전 프레임 */
struct CAnimationPrev
{
	float AnimIndex = 0.f;
	float AnimTime  = 0.f;
	float PlayRate  = 1.f;
};
