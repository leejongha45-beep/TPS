// EnemySpawnParams.h

#pragma once

#include "CoreMinimal.h"

/**
 * SpawnSystem에 전달할 적 생성 파라미터 번들.
 * WaveSubsystem이 DataAsset + 스케일링을 적용하여 채운다.
 */
struct FEnemySpawnParams
{
	FVector Position     = FVector::ZeroVector;
	int32 SwarmID        = INDEX_NONE;
	float MaxHealth      = 0.f;
	float MaxSpeed       = 0.f;
	float AttackDamage   = 0.f;
	float AttackCooldown = 0.f;
	float MeshYawOffset  = 0.f;
};
