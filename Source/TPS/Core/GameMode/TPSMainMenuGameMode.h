#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TPSMainMenuGameMode.generated.h"

/**
 * 메인 메뉴 전용 GameMode
 * - 플레이어 Pawn 없이 메뉴 UI만 표시
 * - HUDClass로 TPSMainMenuHUD를 지정하여 메뉴 위젯 관리
 */
UCLASS()
class TPS_API ATPSMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATPSMainMenuGameMode();
};
