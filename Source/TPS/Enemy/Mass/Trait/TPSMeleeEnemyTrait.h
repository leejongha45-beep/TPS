#pragma once
#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "TPSMeleeEnemyTrait.generated.h"

/**
 * 근접 적 Trait — Entity 생성 시 Fragment 6개 자동 등록
 * - Health, AIState, Movement, Type, LOD, ActorRef
 * - 에디터에서 기본 스탯 조절 가능
 */
UCLASS(meta = (DisplayName = "TPS Melee Enemy"))
class TPS_API UTPSMeleeEnemyTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	/** 기본 최대 체력 */
	UPROPERTY(EditAnywhere, Category = "Stats")
	float DefaultMaxHealth = 50.f;

	/** 기본 이동 속도 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "Stats")
	float DefaultMoveSpeed = 600.f;

	/** 기본 공격 사거리 (cm) */
	UPROPERTY(EditAnywhere, Category = "Stats")
	float DefaultAttackRange = 150.f;

	/** 기본 공격 데미지 */
	UPROPERTY(EditAnywhere, Category = "Stats")
	float DefaultAttackDamage = 10.f;

	/** 기본 공격 간격 (초) */
	UPROPERTY(EditAnywhere, Category = "Stats")
	float DefaultAttackInterval = 1.f;
};
