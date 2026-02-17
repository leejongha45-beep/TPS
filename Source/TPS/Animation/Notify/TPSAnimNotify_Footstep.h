#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "TPSAnimNotify_Footstep.generated.h"

/**
 * 발소리 AnimNotify
 * 로직은 FFootstepHelper에 위임하여
 * 리팩토링 시 이 클래스는 수정 불필요
 */
UCLASS()
class TPS_API UTPSAnimNotify_Footstep : public UAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** 발소리 DataTable (RowType: FFootstepSoundRow) */
	UPROPERTY(EditDefaultsOnly, Category = "Footstep")
	TObjectPtr<class UDataTable> FootstepDataTableAsset;

	/** 발 본 이름 — foot_l 또는 foot_r */
	UPROPERTY(EditDefaultsOnly, Category = "Footstep")
	FName FootBoneName = TEXT("foot_l");

	/** LineTrace 거리 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Footstep", meta = (ClampMin = "10.0", ClampMax = "200.0"))
	float TraceDistance = 50.f;
};
