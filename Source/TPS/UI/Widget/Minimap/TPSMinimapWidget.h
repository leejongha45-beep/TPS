#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPSMinimapWidget.generated.h"

/**
 * 전략 미니맵 위젯
 * - M키 토글로 열기/닫기
 * - MinimapViewModel에서 가공된 데이터만 읽어서 표시 (MVVM)
 * - SwarmSubsystem이 ViewModel을 소유/갱신
 */
UCLASS()
class TPS_API UTPSMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 미니맵 배경 이미지 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> MinimapBackground;

	/** 마커 배치용 캔버스 패널 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> MarkerCanvas;

private:
	/** 정규화 좌표 (0~1) → 위젯 로컬 좌표 변환 */
	FVector2D NormalizedToLocal(const FVector2D& Normalized) const;

	/** 마커 위치 갱신 (CanvasPanel Slot 이용) */
	void UpdateMarkerPosition(class UWidget* Marker, const FVector2D& LocalPos);

	/** 마커 동적 생성 */
	class UImage* CreateMarkerImage(const FLinearColor& Color, float Size);

	/** 동적 군집 마커 풀 */
	UPROPERTY()
	TArray<TObjectPtr<class UImage>> SwarmMarkerPool;

	/** 기지 마커 (동적 생성) */
	UPROPERTY()
	TArray<TObjectPtr<class UImage>> BaseMarkerPool;

	/** 플레이어 마커 (동적 생성) */
	UPROPERTY()
	TObjectPtr<class UImage> PlayerMarker;

	/** 위젯 크기 캐시 */
	FVector2D WidgetSize = FVector2D(600.f, 600.f);

	bool bBasesInitialized = false;
};
