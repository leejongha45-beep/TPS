#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/Player/TPSLinkedAnimInstance.h"
#include "TPSAnimNotify_EquipLayerSwitch.generated.h"

/**
 * 장착 몽타주 중간에 AnimLayer를 전환하는 Notify
 * - 장착 몽타주의 특정 시점에 배치
 * - AnimLayerComponent의 LinkAnimLayer를 호출하여 레이어 교체
 */
UCLASS()
class TPS_API UTPSAnimNotify_EquipLayerSwitch : public UAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** 전환할 대상 AnimLayer BP 클래스 */
	UPROPERTY(EditAnywhere, Category = "Animation")
	TSubclassOf<class UTPSLinkedAnimInstance> TargetAnimLayerClass;
};
