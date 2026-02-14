#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CrosshairViewModel.h"
#include "TPSHUD.generated.h"

UCLASS()
class TPS_API ATPSHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
	FCrosshairConfig CrosshairConfig;

	UPROPERTY()
	TObjectPtr<class UCrosshairViewModel> CrosshairViewModelInst;

	void DrawCrosshair();
	void DrawCrosshairLine(float CenterX, float CenterY,
		float DirectionX, float DirectionY,
		float SpreadOffset, float LineLength,
		float Thickness, const FLinearColor& Color);
	void DrawCenterDot(float CenterX, float CenterY,
		float DotSize, const FLinearColor& Color);
};