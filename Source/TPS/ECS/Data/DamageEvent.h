#pragma once

#include "CoreMinimal.h"

/** 데미지 이벤트 — 프로젝타일 OnHit에서 큐에 적재, DamageSystem에서 소비 */
struct FDamageEvent
{
	int32 InstanceIndex;
	uint8 LODLevel;
	float Damage;
	uint8 bFromPlayer : 1 = false;
	FVector HitLocation = FVector::ZeroVector;
	FVector HitNormal = FVector::ForwardVector;
};

/** 히트 이펙트 요청 — DamageSystem이 생성, 게임 스레드에서 스폰 */
struct FHitEffectRequest
{
	FVector Location;
	FVector Normal;
};
