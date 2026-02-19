#include "UI/Widget/Ammo/TPSAmmoWidget.h"
#include "Components/TextBlock.h"

void UTPSAmmoWidget::UpdateAmmoDisplay(int32 InCurrent, int32 InMax, const FSlateColor& InColor)
{
	if (ensure(CurrentAmmoText))
	{
		CurrentAmmoText->SetText(FText::AsNumber(InCurrent));
		CurrentAmmoText->SetColorAndOpacity(InColor);
	}

	if (ensure(MaxAmmoText))
	{
		MaxAmmoText->SetText(FText::AsNumber(InMax));
	}
}
