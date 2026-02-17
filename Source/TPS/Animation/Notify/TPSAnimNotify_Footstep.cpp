#include "Animation/Notify/TPSAnimNotify_Footstep.h"
#include "Utils/FootstepHelper.h"

void UTPSAnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	FFootstepHelper::PlayFootstepSound(
		MeshComp,
		FootBoneName,
		FootstepDataTableAsset,
		TraceDistance
	);
}
