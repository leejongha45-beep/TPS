#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TPSMainMenuHUD.generated.h"

/**
 * 메인 메뉴 전용 HUD
 * - BeginPlay에서 메뉴 위젯 생성 + UI 입력 모드 설정
 * - 마우스 커서 표시, 게임 입력 차단
 */
UCLASS()
class TPS_API ATPSMainMenuHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	/** 메인 메뉴 위젯 블루프린트 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UTPSMainMenuWidget> MainMenuWidgetClass;

	/** 생성된 메인 메뉴 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<class UTPSMainMenuWidget> MainMenuWidgetInst;
};
