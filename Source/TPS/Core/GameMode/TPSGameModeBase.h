#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TPSGameModeBase.generated.h"

/**
 * 게임 모드 베이스
 * - 기본 Player, Controller, HUD 클래스 지정
 * - 생성자에서 DefaultPawnClass, PlayerControllerClass, HUDClass 설정
 */
UCLASS()
class TPS_API ATPSGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATPSGameModeBase();

protected:
	/** 플레이어 캐릭터 BP 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes")
	TSubclassOf<class ATPSPlayer> BP_PlayerClass;

	/** 플레이어 컨트롤러 BP 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes")
	TSubclassOf<class ATPSPlayerController> BP_PlayerControllerClass;

	/** HUD BP 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes")
	TSubclassOf<class ATPSHUD> BP_HUDClass;
};