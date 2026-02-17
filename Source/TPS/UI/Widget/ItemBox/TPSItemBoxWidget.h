#pragma once

#include "CoreMinimal.h"
#include "Utils/UENUM/WeaponType.h"
#include "Blueprint/UserWidget.h"
#include "TPSItemBoxWidget.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogItemBoxWidget, Log, All);

UCLASS()
class TPS_API UTPSItemBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FORCEINLINE void SetOwningItemBox(class ATPSItemBox* InItemBox) { OwningItemBoxRef = InItemBox; }
	FORCEINLINE void SetOwningPlayer(class ATPSPlayer* InPlayer) { OwningPlayerRef = InPlayer; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnCloseButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CloseButton;

	UFUNCTION()
	void OnRifleButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> RifleButton;

	TWeakObjectPtr<class ATPSItemBox> OwningItemBoxRef;
	TWeakObjectPtr<class ATPSPlayer> OwningPlayerRef;
};