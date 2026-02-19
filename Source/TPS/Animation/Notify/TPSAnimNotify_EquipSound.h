#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "TPSAnimNotify_EquipSound.generated.h"

/**
 * 장비 장착/해제 사운드 AnimNotify
 * - 장착/해제 몽타주 타임라인에 배치하여 사운드 재생
 * - 캐릭터 위치에서 3D 사운드 재생
 */
UCLASS()
class TPS_API UTPSAnimNotify_EquipSound : public UAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** 장착 사운드 에셋 */
	UPROPERTY(EditAnywhere, Category = "Equip")
	TObjectPtr<USoundBase> EquipSoundAsset;
};
