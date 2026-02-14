#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPSInteractionPromptWidget.generated.h"

UCLASS()
class TPS_API UTPSInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPromptText(const FText& InText);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> PromptText;
};
