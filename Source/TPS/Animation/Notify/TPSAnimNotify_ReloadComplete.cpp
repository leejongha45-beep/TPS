#include "Animation/Notify/TPSAnimNotify_ReloadComplete.h"
#include "Character/Component/Action/TPSFireComponent.h"

void UTPSAnimNotify_ReloadComplete::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!ensure(MeshComp)) return;

	AActor* pOwner = MeshComp->GetOwner();
	if (!ensure(pOwner)) return;

	UTPSFireComponent* pFireComp = pOwner->FindComponentByClass<UTPSFireComponent>();
	if (ensure(pFireComp))
	{
		pFireComp->OnReloadNotify();
	}
}
