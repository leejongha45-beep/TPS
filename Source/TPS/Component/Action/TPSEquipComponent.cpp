#include "Component/Action/TPSEquipComponent.h"
#include "GameFramework/Character.h"

DECLARE_LOG_CATEGORY_EXTERN(EquipLog, Log, All);

DEFINE_LOG_CATEGORY(EquipLog);

void UTPSEquipComponent::RequestToggle(bool bIsCurrentlyEquipped)
{
	if (!ensure(WeaponInterface)) return;

	if (bIsEquipTransitioning) return;

	bIsEquipTransitioning = true;


	const bool bEquip = !bIsCurrentlyEquipped;
	UE_LOG(EquipLog, Log, TEXT("[RequestToggle] Request: %s"), bEquip ? TEXT("Equip") : TEXT("Unequip"));

	OnEquipMontagePlayDelegate.Broadcast(bEquip);
}

void UTPSEquipComponent::OnMontageFinished(bool bNewEquippedState)
{
	bIsEquipTransitioning = false;

	UE_LOG(EquipLog, Log, TEXT("[OnMontageFinished] Equipped: %s"), bNewEquippedState ? TEXT("true") : TEXT("false"));

	OnEquipStateChangedDelegate.Broadcast(bNewEquippedState);
}

void UTPSEquipComponent::OnMontageInterrupted()
{
	bIsEquipTransitioning = false;

	UE_LOG(EquipLog, Warning, TEXT("[OnMontageInterrupted] Equip montage was interrupted, transitioning unlocked"));
}

void UTPSEquipComponent::AttachWeapon()
{
	if (!ensure(WeaponInterface)) return;

	const ACharacter* pOwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!ensure(pOwnerCharacter)) return;

	WeaponInterface->Attach(pOwnerCharacter->GetMesh());
}

void UTPSEquipComponent::DetachWeapon()
{
	if (!ensure(WeaponInterface)) return;

	const ACharacter* pOwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!ensure(pOwnerCharacter)) return;

	WeaponInterface->Detach(pOwnerCharacter->GetMesh());
}