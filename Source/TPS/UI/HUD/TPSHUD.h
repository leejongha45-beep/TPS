#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/ViewModel/CrosshairViewModel.h"
#include "TPSHUD.generated.h"

class ATPSGameModeBase;

/**
 * 게임 HUD
 * - Canvas 기반 크로스헤어 드로잉
 * - UMG 기반 탄약/스폰선택 위젯 관리
 * - CrosshairViewModel / AmmoViewModel을 통해 데이터 수신
 * - DrawHUD() 매 프레임 호출
 */
UCLASS()
class TPS_API ATPSHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** 스폰 선택 UI 표시 */
	void ShowSpawnSelect(ATPSGameModeBase* InGameMode);

	/** 스폰 선택 UI 숨김 */
	void HideSpawnSelect();

protected:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

#pragma region Crosshair
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
#pragma endregion

#pragma region Ammo
	/** 탄약 위젯 클래스 (BP에서 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	TSubclassOf<class UTPSAmmoWidget> AmmoWidgetClass;

	/** 탄약 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<class UTPSAmmoWidget> AmmoWidgetInst;

	/** 현재 무기의 AmmoViewModel 참조 */
	TWeakObjectPtr<class UAmmoViewModel> AmmoViewModelRef;

	/** Player 델리게이트 콜백 — 장착/해제 시 ViewModel 전달 */
	void OnAmmoViewModelChanged(class UAmmoViewModel* InAmmoViewModel);

	/** AmmoViewModel → Widget 갱신 */
	void UpdateAmmoWidget();
#pragma endregion

#pragma region SpawnSelect
	/** 스폰 선택 위젯 클래스 (BP에서 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "SpawnSelect")
	TSubclassOf<class UTPSSpawnSelectWidget> SpawnSelectWidgetClass;

	/** 스폰 선택 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<class UTPSSpawnSelectWidget> SpawnSelectWidgetInst;

	/** 스폰 선택 중 여부 — true면 크로스헤어/탄약 숨김 */
	uint8 bIsSpawnSelecting : 1 = false;
#pragma endregion
};