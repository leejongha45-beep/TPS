#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utils/Interface/Action/Attachable.h"
#include "Weapon/TPSWeaponBase.h"
#include "TPSEquipComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquipMontagePlay, bool /* bEquip */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquipStateChanged, bool /* bIsEquipped */);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSEquipComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void RequestToggle(bool bIsCurrentlyEquipped);
	void OnMontageFinished(bool bNewEquippedState);
	void OnMontageInterrupted();
	void AttachWeapon();
	void DetachWeapon();

	FORCEINLINE bool GetIsTransitioning() const { return bIsTransitioning; }
	FORCEINLINE void SetWeaponInterface(TScriptInterface<IAttachable> InWeapon) { WeaponInterface = InWeapon; }
	FORCEINLINE ATPSWeaponBase* GetWeaponActor() const { return Cast<ATPSWeaponBase>(WeaponInterface.GetObject()); }

	FOnEquipMontagePlay OnEquipMontagePlayDelegate;
	FOnEquipStateChanged OnEquipStateChangedDelegate;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TScriptInterface<IAttachable> WeaponInterface;

	uint8 bIsTransitioning : 1 = false;
};