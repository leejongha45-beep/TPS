#include "TPSMainMenuGameMode.h"
#include "TPS/UI/HUD/TPSMainMenuHUD.h"

ATPSMainMenuGameMode::ATPSMainMenuGameMode()
{
	// ① 메뉴 전용 — Pawn 없음
	DefaultPawnClass = nullptr;

	// ② 메뉴 전용 HUD
	HUDClass = ATPSMainMenuHUD::StaticClass();
}
