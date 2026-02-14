#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSItemBoxInteractionComponent.generated.h"

UCLASS()
class TPS_API UTPSItemBoxInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void ToggleItemBox();

	FORCEINLINE bool GetIsOpen() const { return bIsOpen; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UTPSItemBoxWidget> ItemBoxWidgetClass;

	UPROPERTY()
	TObjectPtr<class UTPSItemBoxWidget> ItemBoxWidgetInst;

	uint8 bIsOpen : 1 = false;
};
