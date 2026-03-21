#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPSPsychoSyncWidget.generated.h"

/**
 * 사이코싱크 위젯
 * - 현재 페이즈 텍스트 + 게이지 프로그레스바 표시
 * - 탄약 위젯 바로 위에 배치 (BP에서 앵커/위치 설정)
 * - HUD에서 PsychoSyncViewModel 값을 받아 갱신
 */
UCLASS()
class TPS_API UTPSPsychoSyncWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 게이지 + 페이즈 텍스트 + 색상 갱신 */
	void UpdateDisplay(float InGaugePercent, const FText& InPhaseText, const FLinearColor& InColor);

protected:
	/** BP에서 바인딩 — 게이지 프로그레스바 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> SyncGaugeBar;

	/** BP에서 바인딩 — 페이즈 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> PhaseText;
};
