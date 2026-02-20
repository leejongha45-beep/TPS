#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPSSpawnSelectWidget.generated.h"

class ATPSPlayerStart;
class UTPSSpawnSelectMarkerWidget;

/** 스폰 위치 확정 델리게이트 (선택된 PlayerStart 전달) */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSpawnPointConfirmed, ATPSPlayerStart*);

/**
 * 미니맵 스폰 선택 위젯
 * - 미니맵 배경 위에 플레이어 스폰 포인트 마커를 배치
 * - 마커 클릭 → 하이라이트 → 확인 버튼으로 확정
 * - 월드좌표 → 미니맵 위젯좌표 선형 변환
 */
UCLASS()
class TPS_API UTPSSpawnSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 활성 스폰 포인트 목록을 받아 마커 생성
	 * @param InSpawnPoints  활성 포인트 배열
	 */
	void InitializeSpawnPoints(const TArray<ATPSPlayerStart*>& InSpawnPoints);

	/** 스폰 위치 확정 델리게이트 */
	FOnSpawnPointConfirmed OnSpawnPointConfirmedDelegate;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 미니맵 배경 이미지 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> MinimapImage;

	/** 마커 부모 패널 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> MarkerContainer;

	/** 확인 버튼 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> ConfirmButton;

	/** 선택된 포인트 이름 표시 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> SelectedNameText;

	/** 마커 위젯 클래스 (BP에서 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "SpawnSelect")
	TSubclassOf<UTPSSpawnSelectMarkerWidget> MarkerWidgetClass;

	/** 월드 원점 (월드좌표 → 미니맵 변환용, cm) */
	UPROPERTY(EditDefaultsOnly, Category = "SpawnSelect|Map")
	FVector2D WorldOrigin = FVector2D(-100800.0, -100800.0);

	/** 월드 크기 (cm) — 2016m × 2016m 맵 기준 */
	UPROPERTY(EditDefaultsOnly, Category = "SpawnSelect|Map")
	FVector2D WorldSize = FVector2D(201600.0, 201600.0);

private:
	UFUNCTION()
	void OnConfirmButtonClicked();

	/** 마커 클릭 콜백 */
	void OnMarkerClicked(ATPSPlayerStart* InSpawnPoint);

	/** 월드좌표 → 미니맵 위젯좌표 (0~1 정규화) 변환 */
	FVector2D WorldToMinimap(const FVector& InWorldLocation) const;

	/** 현재 선택된 포인트 */
	TWeakObjectPtr<ATPSPlayerStart> SelectedSpawnPoint;

	/** 생성된 마커 위젯 목록 */
	UPROPERTY()
	TArray<TObjectPtr<UTPSSpawnSelectMarkerWidget>> MarkerWidgets;
};
