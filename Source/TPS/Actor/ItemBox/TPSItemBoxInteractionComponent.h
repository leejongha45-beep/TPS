#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSItemBoxInteractionComponent.generated.h"

UCLASS()
class TPS_API UTPSItemBoxInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void ToggleItemBox(bool bInput, APlayerController* InputController);

	FORCEINLINE bool GetIsOpen() const { return bIsOpening; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UTPSItemBoxWidget> ItemBoxWidgetClass;

	UPROPERTY()
	TObjectPtr<class UTPSItemBoxWidget> ItemBoxWidgetInst;

	uint8 bIsOpening : 1 = false;
};