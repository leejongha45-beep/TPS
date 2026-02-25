// TPSCombatGameMode.cpp

#include "Core/GameMode/TPSCombatGameMode.h"
#include "Wave/TPSWaveSubsystem.h"

void ATPSCombatGameMode::OnSpawnPointSelected(ATPSPlayerStart* InSpawnPoint)
{
	Super::OnSpawnPointSelected(InSpawnPoint);

	// 첫 스폰 완료 시 웨이브 시스템 가동
	if (bFirstSpawnCompleted)
	{
		UTPSWaveSubsystem* pWaveSub = GetWorld()->GetSubsystem<UTPSWaveSubsystem>();
		if (ensure(pWaveSub))
		{
			pWaveSub->StartWaveSystem();
		}
	}
}
