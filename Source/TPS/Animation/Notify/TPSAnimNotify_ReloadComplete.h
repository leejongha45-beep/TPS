#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "TPSAnimNotify_ReloadComplete.generated.h"

/**
 * 재장전 몽타주 중 탄약 충전 시점을 지정하는 Notify
 * - 탄창 교체 애니메이션의 정확한 프레임에 배치
 * - FireComponent::OnReloadNotify()를 호출하여 탄약 충전
 */
UCLASS()
class TPS_API UTPSAnimNotify_ReloadComplete : public UAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
