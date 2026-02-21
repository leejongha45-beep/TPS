#pragma once

#include "Math/Vector.h"
#include "Math/Quat.h"

/**
 * ECS 컴포넌트 — 순수 C++ struct (USTRUCT/UPROPERTY 금지)
 * - EnTT registry에 emplace되는 데이터 단위
 * - UE 타입은 FVector, FQuat 등 값 타입만 허용
 */

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
