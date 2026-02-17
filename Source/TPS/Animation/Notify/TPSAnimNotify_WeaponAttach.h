#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "TPSAnimNotify_WeaponAttach.generated.h"

/**
 * 몽타주 중간에 무기를 부착/분리하는 Notify
 * - 장착 몽타주: 손에 무기 부착 시점에 배치 (bAttach = true)
 * - 해제 몽타주: 무기를 등/홀스터에 분리 시점에 배치 (bAttach = false)
 */
UCLASS()
class TPS_API UTPSAnimNotify_WeaponAttach : public UAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** true=부착 / false=분리 */
	UPROPERTY(EditAnywhere, Category = "Weapon")
	uint8 bAttach : 1 = true;
};
