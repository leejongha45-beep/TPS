#include "Animation/Notify/TPSAnimNotify_EquipLayerSwitch.h"
#include "Component/Action/TPSAnimLayerComponent.h"

void UTPSAnimNotify_EquipLayerSwitch::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!ensure(MeshComp)) return;

	UTPSAnimLayerComponent* AnimLayerComp = MeshComp->GetOwner()->FindComponentByClass<UTPSAnimLayerComponent>();
	if (ensure(AnimLayerComp))
	{
		AnimLayerComp->LinkAnimLayer(TargetAnimLayerClass);
	}
}
