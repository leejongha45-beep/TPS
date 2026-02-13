#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "TPSAnimNotify_EquipLayerSwitch.generated.h"

UCLASS()
class TPS_API UTPSAnimNotify_EquipLayerSwitch : public UAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TSubclassOf<class UTPSLinkedAnimInstance> TargetAnimLayerClass;
};
