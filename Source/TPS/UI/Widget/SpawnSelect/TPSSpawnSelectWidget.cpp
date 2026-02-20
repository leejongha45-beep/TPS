#include "UI/Widget/SpawnSelect/TPSSpawnSelectWidget.h"
#include "UI/Widget/SpawnSelect/TPSSpawnSelectMarkerWidget.h"
#include "Spawn/TPSPlayerStart.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"

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

	// WBP 자식에서 마커 위젯 수집
	CollectMarkerWidgets();
}

void UTPSSpawnSelectWidget::NativeDestruct()
{
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.RemoveAll(this);
	}

	// 마커 델리게이트 정리 (WBP 소유이므로 RemoveFromParent 하지 않음)
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

void UTPSSpawnSelectWidget::CollectMarkerWidgets()
{
	MarkerWidgets.Empty();

	if (!ensure(WidgetTree)) return;

	// WidgetTree에서 모든 자식 순회 → 마커 타입 수집
	WidgetTree->ForEachWidget([this](UWidget* pWidget)
	{
		UTPSSpawnSelectMarkerWidget* pMarker = Cast<UTPSSpawnSelectMarkerWidget>(pWidget);
		if (pMarker)
		{
			MarkerWidgets.Add(pMarker);
		}
	});

	UE_LOG(LogTemp, Log, TEXT("[SpawnSelectWidget] Collected %d markers from WBP"), MarkerWidgets.Num());
}

void UTPSSpawnSelectWidget::InitializeSpawnPoints(const TArray<ATPSPlayerStart*>& InSpawnPoints)
{
	SelectedSpawnPoint = nullptr;

	// ① 각 마커의 DisplayName과 SpawnPoint의 DisplayName을 매칭
	for (UTPSSpawnSelectMarkerWidget* pMarker : MarkerWidgets)
	{
		if (!ensure(pMarker)) continue;

		// 기존 델리게이트 정리
		pMarker->OnMarkerClickedDelegate.RemoveAll(this);

		// DisplayName 매칭 시도
		uint8 bMatched = false;
		for (ATPSPlayerStart* pPoint : InSpawnPoints)
		{
			if (!ensure(pPoint)) continue;

			if (pMarker->GetDisplayName().EqualTo(pPoint->GetDisplayName()))
			{
				// ② 매칭 성공 — SpawnPoint 바인딩 + 델리게이트 연결
				pMarker->SetSpawnPoint(pPoint);
				pMarker->OnMarkerClickedDelegate.AddUObject(this, &UTPSSpawnSelectWidget::OnMarkerClicked);
				bMatched = true;
				break;
			}
		}

		// ③ 매칭 실패 — 마커 비활성화
		if (!bMatched)
		{
			pMarker->SetSpawnPoint(nullptr);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[SpawnSelectWidget] Initialized %d markers with %d spawn points"),
		MarkerWidgets.Num(), InSpawnPoints.Num());
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
