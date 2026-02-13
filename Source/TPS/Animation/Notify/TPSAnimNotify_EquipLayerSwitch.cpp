#include "Animation/Notify/TPSAnimNotify_EquipLayerSwitch.h"
#include "Pawn/Character/Player/TPSPlayer.h"

void UTPSAnimNotify_EquipLayerSwitch::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!ensure(MeshComp)) return;

	ATPSPlayer* pPlayer = Cast<ATPSPlayer>(MeshComp->GetOwner());
	if (ensure(pPlayer))
	{
		pPlayer->LinkAnimLayer(TargetAnimLayerClass);
	}
}
