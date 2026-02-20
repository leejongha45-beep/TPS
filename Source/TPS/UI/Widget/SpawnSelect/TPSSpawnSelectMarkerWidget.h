#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPSSpawnSelectMarkerWidget.generated.h"

class ATPSPlayerStart;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMarkerClicked, ATPSPlayerStart*);

/**
 * 스폰 선택 미니맵 마커 위젯
 * - 개별 플레이어 스폰 포인트를 나타내는 클릭 가능한 마커
 * - 선택 시 시각적 하이라이트
 * - 비활성 포인트는 Disabled 상태
 */
UCLASS()
class TPS_API UTPSSpawnSelectMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 스폰 포인트 바인딩 */
	void SetSpawnPoint(ATPSPlayerStart* InSpawnPoint);

	/** 선택 상태 시각 피드백 */
	void SetSelected(bool bInSelected);

	FORCEINLINE ATPSPlayerStart* GetSpawnPoint() const { return SpawnPointRef.Get(); }

	/** 마커 클릭 델리게이트 */
	FOnMarkerClicked OnMarkerClickedDelegate;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 마커 버튼 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> MarkerButton;

	/** 포인트 이름 (BP 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> NameText;

private:
	UFUNCTION()
	void OnButtonClicked();

	TWeakObjectPtr<ATPSPlayerStart> SpawnPointRef;
};
