#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPSAmmoWidget.generated.h"

/**
 * 탄약 표시 위젯
 * - 현재 탄약 / 최대 탄약 텍스트 표시
 * - HUD에서 AmmoViewModel 값을 받아 갱신
 */
UCLASS()
class TPS_API UTPSAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 탄약 텍스트 갱신 */
	void UpdateAmmoDisplay(int32 InCurrent, int32 InMax);

protected:
	/** BP에서 바인딩할 텍스트 블록 — 현재 탄약 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> CurrentAmmoText;

	/** BP에서 바인딩할 텍스트 블록 — 최대 탄약 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> MaxAmmoText;
};
