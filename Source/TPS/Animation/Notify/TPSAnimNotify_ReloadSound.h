#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "TPSAnimNotify_ReloadSound.generated.h"

/**
 * 재장전 사운드 AnimNotify
 * - 재장전 몽타주 타임라인에 배치하여 사운드 재생
 * - 탄창 분리/삽입 등 복수 배치 가능
 * - 캐릭터 위치에서 3D 사운드 재생
 */
UCLASS()
class TPS_API UTPSAnimNotify_ReloadSound : public UAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** 재장전 사운드 에셋 */
	UPROPERTY(EditAnywhere, Category = "Reload")
	TObjectPtr<USoundBase> ReloadSoundAsset;
};
