#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Sound/SoundBase.h"
#include "TPSAnimNotify_FireSound.generated.h"

/**
 * 총기 사운드 AnimNotify
 * - 발사 몽타주 타임라인에 배치하여 매 발사마다 사운드 재생
 * - Muzzle 소켓 위치에서 3D 사운드 재생
 */
UCLASS()
class TPS_API UTPSAnimNotify_FireSound : public UAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** 총기 사운드 에셋 */
	UPROPERTY(EditAnywhere, Category = "Fire")
	TObjectPtr<USoundBase> FireSoundAsset;

	/** 사운드 재생 위치 기준 소켓 이름 */
	UPROPERTY(EditAnywhere, Category = "Fire")
	FName MuzzleSocketName = TEXT("Muzzle");
};
