#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPSInteractionPromptWidget.generated.h"

/**
 * 상호작용 프롬프트 위젯
 * - "Press F to Interact" 등 텍스트 표시
 * - InteractionComponent에서 Show/Hide 제어
 */
UCLASS()
class TPS_API UTPSInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 프롬프트 텍스트 설정 */
	void SetPromptText(const FText& InText);

protected:
	/** BP에서 바인딩할 텍스트 블록 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> PromptText;
};
