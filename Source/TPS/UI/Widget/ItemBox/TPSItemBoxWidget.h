#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Actor/ItemBox/TPSItemBox.h"
#include "TPSItemBoxWidget.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogItemBoxWidget, Log, All);

UCLASS()
class TPS_API UTPSItemBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FORCEINLINE void SetOwningItemBox(class ATPSItemBox* InItemBox) { OwningItemBoxRef = InItemBox; }

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnCloseButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CloseButton;

	TWeakObjectPtr<class ATPSItemBox> OwningItemBoxRef;
};