#include "Animation/Notify/TPSAnimNotify_WeaponAttach.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/Component/Action/TPSEquipComponent.h"

void UTPSAnimNotify_WeaponAttach::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!ensure(MeshComp)) return;

	AActor* pOwner = MeshComp->GetOwner();
	if (!ensure(pOwner)) return;

	UTPSEquipComponent* pEquipComp = pOwner->FindComponentByClass<UTPSEquipComponent>();
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
