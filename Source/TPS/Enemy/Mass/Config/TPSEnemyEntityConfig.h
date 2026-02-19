#pragma once
#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"
#include "TPSEnemyEntityConfig.generated.h"

/**
 * 적 Mass Entity 설정 DataAsset
 * - 에디터에서 Trait 조합을 세팅
 * - WaveManager가 이 Config를 참조하여 Entity 일괄 생성
 * - 예: MeleeEnemyConfig → TPSMeleeEnemyTrait 포함
 */
UCLASS()
class TPS_API UTPSEnemyEntityConfig : public UMassEntityConfigAsset
{
	GENERATED_BODY()
};
