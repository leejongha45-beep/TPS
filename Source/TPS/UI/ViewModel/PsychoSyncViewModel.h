#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Character/Component/Action/TPSPsychoSyncComponent.h"
#include "PsychoSyncViewModel.generated.h"

/**
 * 사이코싱크 뷰모델
 * - PsychoSyncComponent로부터 페이즈/킬스트릭 데이터 수신
 * - Widget(View)은 Getter로 읽기만 수행
 */
UCLASS()
class TPS_API UPsychoSyncViewModel : public UObject
{
	GENERATED_BODY()

public:
	/** PsychoSyncComponent 바인딩 */
	void Initialize(class UTPSPsychoSyncComponent* InComponent);

	/** 매 프레임 호출 — 게이지 보간 */
	void Update(float DeltaTime);

	FORCEINLINE EPsychoPhase GetCurrentPhase() const { return CurrentPhase; }
	FORCEINLINE float GetGaugePercent() const { return DisplayGaugePercent; }
	FORCEINLINE FText GetPhaseDisplayText() const { return PhaseDisplayText; }
	FORCEINLINE FLinearColor GetGaugeColor() const { return GaugeColor; }
	FORCEINLINE bool IsValid() const { return ComponentRef.IsValid(); }

protected:
	/** 페이즈 변경 콜백 */
	void OnPhaseChanged(EPsychoPhase NewPhase, EPsychoPhase OldPhase);

	/** 내부 값 갱신 */
	void RecalculateGauge();
	void UpdatePhaseDisplay(EPsychoPhase InPhase);

	TWeakObjectPtr<class UTPSPsychoSyncComponent> ComponentRef;

	EPsychoPhase CurrentPhase = EPsychoPhase::Phase0;

	/** 실제 게이지 (0~1) — 다음 페이즈까지의 진행률 */
	float TargetGaugePercent = 0.f;

	/** 보간된 표시용 게이지 */
	float DisplayGaugePercent = 0.f;

	/** 게이지 보간 속도 */
	float GaugeInterpSpeed = 8.f;

	/** 페이즈별 표시 텍스트 */
	FText PhaseDisplayText;

	/** 페이즈별 게이지 색상 */
	FLinearColor GaugeColor = FLinearColor::White;
};
