#pragma once

#include "CoreMinimal.h"

/** 데미지 이벤트 — 프로젝타일 OnHit에서 큐에 적재, DamageSystem에서 소비 */
struct FDamageEvent
{
	int32 InstanceIndex;
	uint8 LODLevel;
	float Damage;
	uint8 bFromPlayer : 1 = false;
};
