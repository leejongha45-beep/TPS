#include "UI/Widget/PsychoSync/TPSPsychoSyncWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UTPSPsychoSyncWidget::UpdateDisplay(float InGaugePercent, const FText& InPhaseText, const FLinearColor& InColor)
{
	if (ensure(SyncGaugeBar.Get()))
	{
		SyncGaugeBar->SetPercent(InGaugePercent);
		SyncGaugeBar->SetFillColorAndOpacity(InColor);
	}

	if (ensure(PhaseText.Get()))
	{
		PhaseText->SetText(InPhaseText);
		PhaseText->SetColorAndOpacity(FSlateColor(InColor));
	}
}
