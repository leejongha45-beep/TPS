// EnemySpawnParams.h

#pragma once

#include "CoreMinimal.h"

/**
 * 적 생성 파라미터 번들.
 * WaveSubsystem이 DataAsset + 스케일링을 적용하여 채운다.
 */
struct FEnemySpawnParams
{
	FVector Position     = FVector::ZeroVector;
	float MaxHealth      = 50.f;
	float MaxSpeed       = 300.f;
	float AttackDamage   = 10.f;
	float AttackCooldown = 1.5f;
};
