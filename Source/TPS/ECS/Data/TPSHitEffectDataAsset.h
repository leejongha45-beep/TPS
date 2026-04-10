// TPSHitEffectDataAsset.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TPSHitEffectDataAsset.generated.h"

/**
 * 피격 이펙트 설정 DataAsset.
 * ECS 적 피격 시 스폰할 Niagara 이펙트를 정의한다.
 */
UCLASS(BlueprintType)
class TPS_API UTPSHitEffectDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 피격 시 스폰할 Niagara 이펙트 */
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSoftObjectPtr<class UNiagaraSystem> HitEffectAsset;

	/** 이펙트 스케일 */
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float EffectScale = 1.0f;
};
