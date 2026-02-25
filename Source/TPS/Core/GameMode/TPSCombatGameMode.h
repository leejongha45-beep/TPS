// TPSCombatGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "Core/GameMode/TPSGameModeBase.h"
#include "TPSCombatGameMode.generated.h"

/**
 * 전투 맵 전용 GameMode.
 * 스폰 선택 완료 후 웨이브 시스템을 가동한다.
 */
UCLASS()
class TPS_API ATPSCombatGameMode : public ATPSGameModeBase
{
	GENERATED_BODY()

public:
	virtual void OnSpawnPointSelected(class ATPSPlayerStart* InSpawnPoint) override;
};
