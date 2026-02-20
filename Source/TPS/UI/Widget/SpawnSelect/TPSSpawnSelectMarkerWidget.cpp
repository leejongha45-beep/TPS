#include "UI/Widget/SpawnSelect/TPSSpawnSelectMarkerWidget.h"
#include "Spawn/TPSPlayerStart.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UTPSSpawnSelectMarkerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ensure(MarkerButton))
	{
		MarkerButton->OnClicked.AddDynamic(this, &UTPSSpawnSelectMarkerWidget::OnButtonClicked);
	}

	// 초기 상태: 선택 안 됨
	SetSelected(false);
}

void UTPSSpawnSelectMarkerWidget::NativeDestruct()
{
	if (MarkerButton)
	{
		MarkerButton->OnClicked.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UTPSSpawnSelectMarkerWidget::SetSpawnPoint(ATPSPlayerStart* InSpawnPoint)
{
	SpawnPointRef = InSpawnPoint;

	if (!InSpawnPoint) return;

	// 이름 표시
	if (ensure(NameText))
	{
		NameText->SetText(InSpawnPoint->GetDisplayName());
	}

	// 비활성 포인트 → 버튼 Disabled
	if (ensure(MarkerButton))
	{
		MarkerButton->SetIsEnabled(InSpawnPoint->IsActive());
	}
}

void UTPSSpawnSelectMarkerWidget::SetSelected(bool bInSelected)
{
	if (!ensure(MarkerButton)) return;

	// 선택 피드백: 버튼 배경색 변경 (BP 스타일 오버라이드)
	FButtonStyle Style = MarkerButton->GetStyle();
	const FLinearColor Color = bInSelected ? FLinearColor(0.2f, 0.6f, 1.f, 1.f) : FLinearColor(0.3f, 0.3f, 0.3f, 0.8f);
	Style.Normal.TintColor = FSlateColor(Color);
	MarkerButton->SetStyle(Style);
}

void UTPSSpawnSelectMarkerWidget::OnButtonClicked()
{
	if (SpawnPointRef.IsValid())
	{
		OnMarkerClickedDelegate.Broadcast(SpawnPointRef.Get());
	}
}
