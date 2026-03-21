#include "UI/ViewModel/PsychoSyncViewModel.h"
#include "Character/Component/Action/TPSPsychoSyncComponent.h"

void UPsychoSyncViewModel::Initialize(class UTPSPsychoSyncComponent* InComponent)
{
	if (!ensure(InComponent)) return;

	ComponentRef = InComponent;
	CurrentPhase = InComponent->GetCurrentPhase();

	UpdatePhaseDisplay(CurrentPhase);
	RecalculateGauge();
	DisplayGaugePercent = TargetGaugePercent;

	// 페이즈 변경 델리게이트 바인딩
	InComponent->OnPsychoPhaseChangedDelegate.AddUObject(this, &UPsychoSyncViewModel::OnPhaseChanged);
}

void UPsychoSyncViewModel::Update(float DeltaTime)
{
	if (!ComponentRef.IsValid()) return;

	RecalculateGauge();

	// 게이지 부드럽게 보간
	DisplayGaugePercent = FMath::FInterpTo(DisplayGaugePercent, TargetGaugePercent, DeltaTime, GaugeInterpSpeed);
}

void UPsychoSyncViewModel::OnPhaseChanged(EPsychoPhase NewPhase, EPsychoPhase OldPhase)
{
	CurrentPhase = NewPhase;
	UpdatePhaseDisplay(NewPhase);
}

void UPsychoSyncViewModel::RecalculateGauge()
{
	UTPSPsychoSyncComponent* pComp = ComponentRef.Get();
	if (!pComp) return;

	const int32 KillStreak = pComp->GetKillStreak();
	const int32 Phase1Threshold = 20;
	const int32 Phase2Threshold = 50;

	switch (CurrentPhase)
	{
	case EPsychoPhase::Phase0:
		// 0 ~ Phase1Threshold 구간
		TargetGaugePercent = FMath::Clamp(static_cast<float>(KillStreak) / Phase1Threshold, 0.f, 1.f);
		break;

	case EPsychoPhase::Phase1_InfiniteAmmo:
		// Phase1Threshold ~ Phase2Threshold 구간
		TargetGaugePercent = FMath::Clamp(
			static_cast<float>(KillStreak - Phase1Threshold) / (Phase2Threshold - Phase1Threshold), 0.f, 1.f);
		break;

	case EPsychoPhase::Phase2_Penetration:
		// 최대 페이즈 — 게이지 풀
		TargetGaugePercent = 1.f;
		break;
	}
}

void UPsychoSyncViewModel::UpdatePhaseDisplay(EPsychoPhase InPhase)
{
	switch (InPhase)
	{
	case EPsychoPhase::Phase0:
		PhaseDisplayText = FText::FromString(TEXT("SYNC OFF"));
		GaugeColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.f);	// 회색
		break;

	case EPsychoPhase::Phase1_InfiniteAmmo:
		PhaseDisplayText = FText::FromString(TEXT("INFINITE AMMO"));
		GaugeColor = FLinearColor(0.2f, 0.6f, 1.f, 1.f);	// 파란색
		break;

	case EPsychoPhase::Phase2_Penetration:
		PhaseDisplayText = FText::FromString(TEXT("PENETRATION"));
		GaugeColor = FLinearColor(1.f, 0.3f, 0.1f, 1.f);	// 빨간/주황
		break;
	}
}
