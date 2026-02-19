#pragma once
#include "CoreMinimal.h"
#include "EnemyLODLevel.generated.h"

/**
 * 적 LOD 단계
 * - None:      150m+ — 데이터만 (렌더링 없음)
 * - ISM:       50~150m — InstancedStaticMesh 간이 렌더
 * - FullActor: 50m 이내 — APawn + SkeletalMesh + 충돌
 */
UENUM(BlueprintType)
enum class EEnemyLODLevel : uint8
{
	None      UMETA(DisplayName = "None"),
	ISM       UMETA(DisplayName = "ISM"),
	FullActor UMETA(DisplayName = "FullActor"),
};
