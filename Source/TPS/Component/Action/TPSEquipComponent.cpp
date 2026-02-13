#include "Component/Action/TPSEquipComponent.h"

DECLARE_LOG_CATEGORY_EXTERN(EquipLog, Log, All);
DEFINE_LOG_CATEGORY(EquipLog);

void UTPSEquipComponent::RequestToggle(bool bIsCurrentlyEquipped)
{
	if (bIsTransitioning) return;

	bIsTransitioning = true;

	const bool bEquip = !bIsCurrentlyEquipped;
	UE_LOG(EquipLog, Log, TEXT("[RequestToggle] Request: %s"), bEquip ? TEXT("Equip") : TEXT("Unequip"));

	OnEquipMontagePlayDelegate.Broadcast(bEquip);
}

void UTPSEquipComponent::OnMontageFinished(bool bNewEquippedState)
{
	bIsTransitioning = false;

	UE_LOG(EquipLog, Log, TEXT("[OnMontageFinished] Equipped: %s"), bNewEquippedState ? TEXT("true") : TEXT("false"));

	OnEquipStateChangedDelegate.Broadcast(bNewEquippedState);
}
