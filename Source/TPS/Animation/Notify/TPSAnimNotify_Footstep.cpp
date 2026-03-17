#include "Animation/Notify/TPSAnimNotify_Footstep.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/Component/Data/TPSFootstepComponent.h"

void UTPSAnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	AActor* pOwner = MeshComp->GetOwner();
	if (!pOwner) return;

	UTPSFootstepComponent* pFootstepComp =
		pOwner->FindComponentByClass<UTPSFootstepComponent>();

	if (!ensure(pFootstepComp)) return;

	pFootstepComp->PlayFootstepSound(MeshComp, FootBoneName, TraceDistance);
}
