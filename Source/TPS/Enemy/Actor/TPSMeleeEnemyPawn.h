#pragma once
#include "CoreMinimal.h"
#include "Enemy/Actor/TPSEnemyPawnBase.h"
#include "TPSMeleeEnemyPawn.generated.h"

/**
 * 근접 적 폰
 * - ATPSEnemyPawnBase 상속
 * - 근접 고속 군집형 전용 메시/애니메이션
 * - 실제 에셋은 BP에서 EditDefaultsOnly로 지정
 */
UCLASS()
class TPS_API ATPSMeleeEnemyPawn : public ATPSEnemyPawnBase
{
	GENERATED_BODY()

public:
	ATPSMeleeEnemyPawn();

protected:
	/** 근접 공격 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> AttackMontageAsset;

	/** 사망 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> DeathMontageAsset;
};
