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
	AttackCooldown,   // 공격 범위 내 쿨다운 대기
	AttackReady,
	Attacking,
	Dying,     // 사망 애니메이션 재생 중
	Dead       // 애니메이션 완료 → Cleanup 대상
};

/** LOD 레벨 — 거리 기반 틱 빈도 제어 */
enum class ELODLevel : uint8
{
	Near,   // 0~30000u (300m) — 매 프레임
	Mid,    // 30000~50000u (500m) — 2프레임 주기
	Far     // 50000u+ — 4프레임 주기
};

/** ECS 상수 — 시스템 간 공유 */
namespace ECSConstants
{
	constexpr float DeathAnimDuration = 2.f;

	// ── Separation (겹침 방지) ──
	constexpr float SeparationRadius          = 150.f;
	constexpr float SeparationRadiusSq        = SeparationRadius * SeparationRadius;
	constexpr float SeparationWeight          = 2.5f;
	constexpr float MaxSeparationForce        = 800.f;
	constexpr float SeparationCullingRadius   = 3000.f;
	constexpr float SeparationCullingRadiusSq = SeparationCullingRadius * SeparationCullingRadius;

	// ── Attack (공격 수행) ──
	constexpr float AttackDamage          = 10.f;
	constexpr float AttackCooldown        = 1.5f;
	constexpr float AttackReadyDuration   = 0.5f;
	constexpr float AttackDuration        = 0.3f;

	// ── LOD (거리 기반 틱 빈도 제어) ──
	constexpr float LODNearRadius       = 30000.f;                         // 300m
	constexpr float LODNearRadiusSq     = LODNearRadius * LODNearRadius;
	constexpr float LODMidRadius        = 50000.f;                         // 500m
	constexpr float LODMidRadiusSq      = LODMidRadius * LODMidRadius;
	constexpr int32 LODNearTickInterval = 1;   // 매 프레임
	constexpr int32 LODMidTickInterval  = 2;   // 2프레임 주기
	constexpr int32 LODFarTickInterval  = 4;   // 4프레임 주기

	// ── LOD 히스테리시스 (경계 진동 방지) ──
	constexpr float LODHysteresisMargin   = 2000.f;                                        // 20m
	constexpr float LODMidToNearRadiusSq  = (LODNearRadius - LODHysteresisMargin)           // 28000²
	                                      * (LODNearRadius - LODHysteresisMargin);
	constexpr float LODFarToMidRadiusSq   = (LODMidRadius  - LODHysteresisMargin)           // 48000²
	                                      * (LODMidRadius  - LODHysteresisMargin);
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

/** 렌더 프록시 — HISM 인스턴스 인덱스 + LOD 소속 */
struct CRenderProxy
{
	int32 InstanceIndex = INDEX_NONE;
	uint8 LODLevel = 0;   // 현재 소속 HISM의 LOD 인덱스 (0=Near, 1=Mid, 2=Far)
};

/** 애니메이션 — VAT 재생 상태 */
struct CAnimation
{
	float AnimIndex = 0.f;
	float AnimTime  = 0.f;
	float PlayRate  = 1.f;
};

/** 공격 — 쿨다운 기반 데미지 수행 */
struct CAttack
{
	float Damage        = 10.f;
	float Cooldown      = 1.5f;
	float CooldownTimer = 0.f;
};

/** LOD — 거리 기반 틱 빈도 제어 */
struct CLOD
{
	ELODLevel Level = ELODLevel::Near;
	uint8 FrameOffset = 0;                  // Entity별 프레임 오프셋 (부하 분산)
	int32 TickInterval = 1;                 // 현재 LOD의 틱 주기
	float AccumulatedDeltaTime = 0.f;       // 스킵 프레임 누적 시간
	uint8 bShouldTick : 1 = true;           // 이번 프레임 틱 여부
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
	uint8 LODLevel = 0;
};

/** 시각화 캐시 — HISM에 마지막으로 기록한 값 (변경 감지용) */
struct CVisCache
{
	FVector Position = FVector(ForceInit);
	FQuat Rotation = FQuat::Identity;
	float AnimIndex = -1.f;   // 초기값 -1 → 첫 프레임 반드시 갱신
	float AnimTime  = -1.f;
};

/** 시각화 캐시 — 이전 프레임 */
struct CVisCachePrev
{
	FVector Position = FVector(ForceInit);
	FQuat Rotation = FQuat::Identity;
	float AnimIndex = -1.f;
	float AnimTime  = -1.f;
};

/** 애니메이션 — 이전 프레임 */
struct CAnimationPrev
{
	float AnimIndex = 0.f;
	float AnimTime  = 0.f;
	float PlayRate  = 1.f;
};

/** 공격 — 이전 프레임 */
struct CAttackPrev
{
	float Damage        = 10.f;
	float Cooldown      = 1.5f;
	float CooldownTimer = 0.f;
};

/** LOD — 이전 프레임 */
struct CLODPrev
{
	ELODLevel Level = ELODLevel::Near;
	uint8 FrameOffset = 0;
	int32 TickInterval = 1;
	float AccumulatedDeltaTime = 0.f;
	uint8 bShouldTick : 1 = true;
};

