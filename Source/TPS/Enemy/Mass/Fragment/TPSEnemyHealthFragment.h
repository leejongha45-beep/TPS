#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "TPSEnemyHealthFragment.generated.h"

/**
 * 적 체력 Fragment
 * - Mass Entity 데이터 레이어에서 체력 관리
 * - FullActor LOD 시 HealthComponent ↔ Fragment 양방향 동기화
 * - Actor 없는 먼 적은 Processor가 이 Fragment 직접 조작
 */
USTRUCT()
struct FTPSEnemyHealthFragment : public FMassFragment
{
	GENERATED_BODY()

	/** 현재 체력 */
	float CurrentHealth = 0.f;

	/** 최대 체력 */
	float MaxHealth = 0.f;
};
