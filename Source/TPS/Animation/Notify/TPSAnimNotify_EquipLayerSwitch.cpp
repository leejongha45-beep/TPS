#include "Animation/Notify/TPSAnimNotify_EquipLayerSwitch.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/Component/Data/TPSAnimLayerComponent.h"

void UTPSAnimNotify_EquipLayerSwitch::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!ensure(MeshComp)) return;

	AActor* pOwner = MeshComp->GetOwner();
	if (!ensure(pOwner)) return;

	UTPSAnimLayerComponent* pAnimLayerComp = pOwner->FindComponentByClass<UTPSAnimLayerComponent>();
	if (ensure(pAnimLayerComp))
	{
		pAnimLayerComp->LinkAnimLayer(TargetAnimLayerClass);
	}
}
