#include "Animation/Notify/TPSAnimNotify_WeaponAttach.h"
#include "Component/Action/TPSEquipComponent.h"

void UTPSAnimNotify_WeaponAttach::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!ensure(MeshComp)) return;

	UTPSEquipComponent* pEquipComp = MeshComp->GetOwner()->FindComponentByClass<UTPSEquipComponent>();
	if (ensure(pEquipComp))
	{
		if (bAttach)
		{
			pEquipComp->AttachWeapon();
		}
		else
		{
			pEquipComp->DetachWeapon();
		}
	}
}
