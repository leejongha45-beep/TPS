#include "TPSMainMenuHUD.h"
#include "TPS/UI/Widget/MainMenu/TPSMainMenuWidget.h"
#include "GameFramework/PlayerController.h"

void ATPSMainMenuHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* pPC = GetOwningPlayerController();
	if (!ensure(pPC)) return;

	// ① 메뉴 위젯 생성
	if (ensure(MainMenuWidgetClass))
	{
		MainMenuWidgetInst = CreateWidget<UTPSMainMenuWidget>(pPC, MainMenuWidgetClass);
		if (ensure(MainMenuWidgetInst))
		{
			MainMenuWidgetInst->AddToViewport();
		}
	}

	// ② UI 전용 입력 모드 + 마우스 커서 표시
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	pPC->SetInputMode(InputMode);
	pPC->bShowMouseCursor = true;
}
