#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TPSGameModeBase.generated.h"

/**
 * 게임 모드 베이스
 * - 기본 Player, Controller, HUD 클래스 지정
 * - 생성자에서 DefaultPawnClass, PlayerControllerClass, HUDClass 설정
 * - WaveManager 소유 + BeginPlay에서 웨이브 시작
 */
UCLASS()
class TPS_API ATPSGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATPSGameModeBase();

	FORCEINLINE class UTPSWaveManager* GetWaveManager() const { return WaveManagerInst; }

protected:
	virtual void BeginPlay() override;

#pragma region Wave
	/** 웨이브 설정 DataAsset (에디터에서 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "Wave")
	TObjectPtr<class UTPSWaveConfig> WaveConfig;

	/** 웨이브 매니저 (런타임 생성) */
	UPROPERTY()
	TObjectPtr<class UTPSWaveManager> WaveManagerInst;
#pragma endregion
};