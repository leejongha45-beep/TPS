#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPSSpawnSelectWidget.generated.h"

/** 스폰 위치 확정 델리게이트 (선택된 PlayerStart 전달) */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSpawnPointConfirmed, class ATPSPlayerStart*);

/**
 * 미니맵 스폰 선택 위젯
 * - 미니맵 배경 위에 디자이너가 WBP에서 직접 배치한 마커를 사용
 * - NativeConstruct에서 자식 위젯 중 마커 타입을 자동 수집
 * - InitializeSpawnPoints()에서 DisplayName 매칭으로 SpawnPoint 연결
 * - 마커 클릭 → 하이라이트 → 확인 버튼으로 확정
 */
UCLASS()
class TPS_API UTPSSpawnSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 활성 스폰 포인트를 WBP 마커와 DisplayName으로 매칭
	 * @param InSpawnPoints  활성 포인트 배열
	 */
	void InitializeSpawnPoints(const TArray<class ATPSPlayerStart*>& InSpawnPoints);

	/** 스폰 위치 확정 델리게이트 */
	FOnSpawnPointConfirmed OnSpawnPointConfirmedDelegate;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 미니맵 배경 이미지 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> MinimapImage;

	/** 확인 버튼 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> ConfirmButton;

	/** 선택된 포인트 이름 표시 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> SelectedNameText;

private:
	UFUNCTION()
	void OnConfirmButtonClicked();

	/** 마커 클릭 콜백 */
	void OnMarkerClicked(class ATPSPlayerStart* InSpawnPoint);

	/** WBP 자식에서 마커 위젯 자동 수집 */
	void CollectMarkerWidgets();

	/** 현재 선택된 포인트 */
	TWeakObjectPtr<class ATPSPlayerStart> SelectedSpawnPoint;

	/** WBP에서 수집된 마커 위젯 목록 */
	UPROPERTY()
	TArray<TObjectPtr<class UTPSSpawnSelectMarkerWidget>> MarkerWidgets;
};