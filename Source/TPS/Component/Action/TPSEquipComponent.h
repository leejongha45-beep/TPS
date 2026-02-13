#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

	FORCEINLINE bool GetIsTransitioning() const { return bIsTransitioning; }

	FOnEquipMontagePlay OnEquipMontagePlayDelegate;
	FOnEquipStateChanged OnEquipStateChangedDelegate;

protected:
	uint8 bIsTransitioning : 1 = false;
};
