#include "Core/GameMode/TPSGameModeBase.h"

#include "Core/Controller/TPSPlayerController.h"
#include "Pawn/Character/Player/TPSPlayer.h"
#include "UI/HUD/TPSHUD.h"
#include "Wave/TPSWaveManager.h"
#include "Wave/TPSWaveConfig.h"
#include "Enemy/Mass/Processor/TPSEnemyDeathProcessor.h"

ATPSGameModeBase::ATPSGameModeBase()
{
}

void ATPSGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// 웨이브 매니저 초기화 + 시작
	if (ensure(WaveConfig))
	{
		WaveManagerInst = NewObject<UTPSWaveManager>(this);
		if (ensure(WaveManagerInst))
		{
			WaveManagerInst->Initialize(GetWorld(), WaveConfig);
			WaveManagerInst->StartWaves();

			// 킬 델리게이트 바인딩 — DeathProcessor → WaveManager
			UTPSEnemyDeathProcessor::OnEnemyKilledDelegate.AddUObject(
				WaveManagerInst, &UTPSWaveManager::NotifyEnemyKilled);
		}
	}
}