#include "UI/Widget/Minimap/TPSMinimapWidget.h"
#include "UI/ViewModel/MinimapViewModel.h"
#include "Core/Subsystem/TPSSwarmSubsystem.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/Texture2D.h"

void UTPSMinimapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bBasesInitialized = false;

	// 플레이어 마커 동적 생성 (초록)
	PlayerMarker = CreateMarkerImage(FLinearColor::Green, 14.f);
}

void UTPSMinimapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UWorld* pWorld = GetWorld();
	if (!pWorld) return;

	UTPSSwarmSubsystem* pSwarmSS = pWorld->GetSubsystem<UTPSSwarmSubsystem>();
	if (!pSwarmSS) return;

	UMinimapViewModel* pVM = pSwarmSS->GetMinimapViewModel();
	if (!pVM) return;

	// ① 위젯 크기 캐시
	if (MinimapBackground.Get())
	{
		const FVector2D BgSize = MinimapBackground->GetCachedGeometry().GetLocalSize();
		if (BgSize.X > 0.f && BgSize.Y > 0.f)
		{
			WidgetSize = BgSize;
		}
	}

	// ② 기지 마커 초기화 (1회)
	if (!bBasesInitialized)
	{
		const TArray<FMinimapMarkerData>& BaseData = pVM->GetBaseMarkers();
		if (BaseData.Num() > 0)
		{
			for (const FMinimapMarkerData& Data : BaseData)
			{
				UImage* Marker = CreateMarkerImage(Data.Color, Data.Size);
				if (Marker)
				{
					BaseMarkerPool.Add(Marker);
					UpdateMarkerPosition(Marker, NormalizedToLocal(Data.Position));
				}
			}
			bBasesInitialized = true;
		}
	}

	// ③ 군집 마커 갱신
	const TArray<FMinimapMarkerData>& SwarmData = pVM->GetSwarmMarkers();

	// 마커 풀 확장
	while (SwarmMarkerPool.Num() < SwarmData.Num())
	{
		UImage* Marker = CreateMarkerImage(FLinearColor::White, 10.f);
		if (Marker)
		{
			SwarmMarkerPool.Add(Marker);
		}
	}

	for (int32 i = 0; i < SwarmData.Num(); ++i)
	{
		const FMinimapMarkerData& Data = SwarmData[i];
		UImage* Marker = SwarmMarkerPool[i].Get();
		if (!Marker) continue;

		if (!Data.bVisible)
		{
			Marker->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}

		Marker->SetVisibility(ESlateVisibility::HitTestInvisible);
		Marker->SetColorAndOpacity(Data.Color);

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Marker->Slot))
		{
			CanvasSlot->SetSize(FVector2D(Data.Size, Data.Size));
		}

		UpdateMarkerPosition(Marker, NormalizedToLocal(Data.Position));
	}

	// 초과 마커 숨기기
	for (int32 i = SwarmData.Num(); i < SwarmMarkerPool.Num(); ++i)
	{
		if (SwarmMarkerPool[i].Get())
		{
			SwarmMarkerPool[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// ④ 플레이어 마커 갱신
	if (PlayerMarker.Get() && pVM->IsPlayerValid())
	{
		UpdateMarkerPosition(PlayerMarker.Get(), NormalizedToLocal(pVM->GetPlayerPosition()));
	}
}

FVector2D UTPSMinimapWidget::NormalizedToLocal(const FVector2D& Normalized) const
{
	return FVector2D(Normalized.X * WidgetSize.X, Normalized.Y * WidgetSize.Y);
}

void UTPSMinimapWidget::UpdateMarkerPosition(UWidget* Marker, const FVector2D& LocalPos)
{
	if (!Marker) return;

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Marker->Slot);
	if (CanvasSlot)
	{
		CanvasSlot->SetPosition(LocalPos);
	}
}

UImage* UTPSMinimapWidget::CreateMarkerImage(const FLinearColor& Color, float Size)
{
	if (!MarkerCanvas.Get()) return nullptr;

	UImage* NewImage = NewObject<UImage>(this);
	if (!NewImage) return nullptr;

	// 엔진 WhiteSquareTexture로 솔리드 브러시 → Color 틴트로 색상 표현
	UTexture2D* WhiteTex = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture"));
	if (WhiteTex)
	{
		NewImage->SetBrushFromTexture(WhiteTex);
	}
	NewImage->SetColorAndOpacity(Color);

	UCanvasPanelSlot* NewSlot = MarkerCanvas->AddChildToCanvas(NewImage);
	if (NewSlot)
	{
		NewSlot->SetSize(FVector2D(Size, Size));
		NewSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		NewSlot->SetAutoSize(false);
	}

	return NewImage;
}
