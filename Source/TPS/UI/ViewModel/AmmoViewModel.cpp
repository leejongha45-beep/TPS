#include "UI/ViewModel/AmmoViewModel.h"

void UAmmoViewModel::SetAmmo(int32 InCurrent, int32 InMax)
{
	CurrentAmmo = InCurrent;
	MaxAmmo = InMax;
	UpdateAmmoColor();
}

void UAmmoViewModel::UpdateAmmoColor()
{
	AmmoColor = (CurrentAmmo >= LowAmmoThreshold)
		? FSlateColor(FLinearColor(0.2f, 0.4f, 1.f))
		: FSlateColor(FLinearColor(1.f, 0.2f, 0.2f));
}
