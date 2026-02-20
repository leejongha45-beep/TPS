#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TPSVillageGameMode.generated.h"

/**
 * 마을 전용 GameMode
 * - 전투 로직 없음 (웨이브/스폰 없음)
 * - 무기 장착, 스테이지 선택 UI 관리
 * - 플레이어는 자유 이동 가능
 */
UCLASS()
class TPS_API ATPSVillageGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATPSVillageGameMode();
};
