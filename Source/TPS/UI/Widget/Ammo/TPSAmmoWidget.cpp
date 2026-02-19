#include "UI/Widget/Ammo/TPSAmmoWidget.h"
#include "Components/TextBlock.h"

void UTPSAmmoWidget::UpdateAmmoDisplay(int32 InCurrent, int32 InMax)
{
	if (ensure(CurrentAmmoText))
	{
		CurrentAmmoText->SetText(FText::AsNumber(InCurrent));
	}

	if (ensure(MaxAmmoText))
	{
		MaxAmmoText->SetText(FText::AsNumber(InMax));
	}
}
