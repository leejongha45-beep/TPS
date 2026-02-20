#include "UI/Widget/SpawnSelect/TPSSpawnSelectWidget.h"
#include "UI/Widget/SpawnSelect/TPSSpawnSelectMarkerWidget.h"
#include "Spawn/TPSPlayerStart.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UTPSSpawnSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ensure(ConfirmButton))
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UTPSSpawnSelectWidget::OnConfirmButtonClicked);
		ConfirmButton->SetIsEnabled(false);
	}

	if (ensure(SelectedNameText))
	{
		SelectedNameText->SetText(FText::FromString(TEXT("스폰 위치를 선택하세요")));
	}
}

void UTPSSpawnSelectWidget::NativeDestruct()
{
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.RemoveAll(this);
	}

	// 마커 델리게이트 정리
	for (UTPSSpawnSelectMarkerWidget* pMarker : MarkerWidgets)
	{
		if (pMarker)
		{
			pMarker->OnMarkerClickedDelegate.RemoveAll(this);
		}
	}
	MarkerWidgets.Empty();

	Super::NativeDestruct();
}

void UTPSSpawnSelectWidget::InitializeSpawnPoints(const TArray<ATPSPlayerStart*>& InSpawnPoints)
{
	if (!ensure(MarkerContainer)) return;
	if (!ensure(MarkerWidgetClass)) return;

	// 기존 마커 정리
	for (UTPSSpawnSelectMarkerWidget* pMarker : MarkerWidgets)
	{
		if (pMarker)
		{
			pMarker->OnMarkerClickedDelegate.RemoveAll(this);
			pMarker->RemoveFromParent();
		}
	}
	MarkerWidgets.Empty();
	SelectedSpawnPoint = nullptr;

	APlayerController* pPC = GetOwningPlayer();
	if (!ensure(pPC)) return;

	for (ATPSPlayerStart* pPoint : InSpawnPoints)
	{
		if (!ensure(pPoint)) continue;

		UTPSSpawnSelectMarkerWidget* pMarker = CreateWidget<UTPSSpawnSelectMarkerWidget>(pPC, MarkerWidgetClass);
		if (!ensure(pMarker)) continue;

		pMarker->SetSpawnPoint(pPoint);
		pMarker->OnMarkerClickedDelegate.AddUObject(this, &UTPSSpawnSelectWidget::OnMarkerClicked);

		// MarkerContainer (CanvasPanel)에 추가
		UCanvasPanelSlot* pSlot = MarkerContainer->AddChildToCanvas(pMarker);
		if (ensure(pSlot))
		{
			// 월드좌표 → 미니맵 정규화 좌표
			const FVector2D NormPos = WorldToMinimap(pPoint->GetActorLocation());

			// 정규화 좌표를 앵커로 사용 (미니맵 크기에 자동 비례)
			pSlot->SetAnchors(FAnchors(NormPos.X, NormPos.Y, NormPos.X, NormPos.Y));
			pSlot->SetAlignment(FVector2D(0.5, 0.5));
			pSlot->SetAutoSize(true);
		}

		MarkerWidgets.Add(pMarker);
	}

	UE_LOG(LogTemp, Log, TEXT("[SpawnSelectWidget] Created %d markers"), MarkerWidgets.Num());
}

void UTPSSpawnSelectWidget::OnMarkerClicked(ATPSPlayerStart* InSpawnPoint)
{
	if (!InSpawnPoint) return;

	SelectedSpawnPoint = InSpawnPoint;

	// 모든 마커 선택 해제 → 클릭된 마커만 선택
	for (UTPSSpawnSelectMarkerWidget* pMarker : MarkerWidgets)
	{
		if (ensure(pMarker))
		{
			pMarker->SetSelected(pMarker->GetSpawnPoint() == InSpawnPoint);
		}
	}

	// 선택 이름 표시 + 확인 버튼 활성화
	if (ensure(SelectedNameText))
	{
		SelectedNameText->SetText(InSpawnPoint->GetDisplayName());
	}

	if (ensure(ConfirmButton))
	{
		ConfirmButton->SetIsEnabled(true);
	}
}

void UTPSSpawnSelectWidget::OnConfirmButtonClicked()
{
	if (!SelectedSpawnPoint.IsValid()) return;

	OnSpawnPointConfirmedDelegate.Broadcast(SelectedSpawnPoint.Get());
}

FVector2D UTPSSpawnSelectWidget::WorldToMinimap(const FVector& InWorldLocation) const
{
	// 월드좌표를 0~1 범위로 정규화
	const double NormX = (InWorldLocation.X - WorldOrigin.X) / WorldSize.X;
	const double NormY = (InWorldLocation.Y - WorldOrigin.Y) / WorldSize.Y;

	return FVector2D(
		FMath::Clamp(NormX, 0.0, 1.0),
		FMath::Clamp(NormY, 0.0, 1.0)
	);
}
