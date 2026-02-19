#pragma once
#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Utils/UENUM/EnemyType.h"
#include "TPSEnemyTypeFragment.generated.h"

/**
 * 적 타입 Fragment
 * - 적 종류 식별 (Melee/Ranged/Heavy)
 * - Processor가 타입별 행동 분기
 * - LODProcessor가 타입별 메시/ISM 매핑
 * - WaveManager가 스폰 시 타입 지정
 */
USTRUCT()
struct FTPSEnemyTypeFragment : public FMassFragment
{
	GENERATED_BODY()

	/** 적 종류 */
	EEnemyType EnemyType = EEnemyType::Melee;
};
