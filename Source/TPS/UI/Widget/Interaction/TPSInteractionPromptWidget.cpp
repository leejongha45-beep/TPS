#include "UI/Widget/Interaction/TPSInteractionPromptWidget.h"
#include "Components/TextBlock.h"

void UTPSInteractionPromptWidget::SetPromptText(const FText& InText)
{
	if (ensure(PromptText))
	{
		PromptText->SetText(InText);
	}
}
