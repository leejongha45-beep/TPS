#pragma once
#include "CoreMinimal.h"
#include "EnemyType.generated.h"

/**
 * 적 타입 분류
 * - 웨이브 스폰, Actor 풀, Trait 구분에 사용
 */
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Melee  = 0,   /** 근접 고속 군집 */
	Ranged = 1,   /** 원거리 혼합 */
	Heavy  = 2,   /** 보스급 탱커 */
};
