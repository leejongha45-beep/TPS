#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CrosshairViewModel.h"
#include "TPSHUD.generated.h"

/**
 * 게임 HUD
 * - Canvas 기반 크로스헤어 드로잉
 * - CrosshairViewModel을 통해 스프레드/스타일 관리
 * - DrawHUD() 매 프레임 호출
 */
UCLASS()
class TPS_API ATPSHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	/** 크로스헤어 설정 (스프레드, 스타일) */
	UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
	FCrosshairConfig CrosshairConfig;

	/** 크로스헤어 뷰모델 인스턴스 */
	UPROPERTY()
	TObjectPtr<class UCrosshairViewModel> CrosshairViewModelInst;

	/** 크로스헤어 전체 드로잉 */
	void DrawCrosshair();

	/** 크로스헤어 라인 1개 드로잉 (상/하/좌/우) */
	void DrawCrosshairLine(float CenterX, float CenterY,
		float DirectionX, float DirectionY,
		float SpreadOffset, float LineLength,
		float Thickness, const FLinearColor& Color);

	/** 중앙 도트 드로잉 */
	void DrawCenterDot(float CenterX, float CenterY,
		float DotSize, const FLinearColor& Color);
};