#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "TPSAnimNotify_Footstep.generated.h"

/**
 * 발소리 AnimNotify
 * - Owner의 UTPSFootstepComponent에 재생 위임
 * - 본 이름과 트레이스 거리만 보유 (DataTable은 Component가 캐싱)
 */
UCLASS()
class TPS_API UTPSAnimNotify_Footstep : public UAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** 발 본 이름 — foot_l 또는 foot_r */
	UPROPERTY(EditDefaultsOnly, Category = "Footstep")
	FName FootBoneName = TEXT("foot_l");

	/** LineTrace 거리 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Footstep", meta = (ClampMin = "10.0", ClampMax = "200.0"))
	float TraceDistance = 50.f;
};
