// TPSCombatGameMode.cpp

#include "Core/GameMode/TPSCombatGameMode.h"
#include "Engine/World.h"
#include "Core/Subsystem/TPSSwarmSubsystem.h"

void ATPSCombatGameMode::OnSpawnPointSelected(ATPSPlayerStart* InSpawnPoint)
{
	Super::OnSpawnPointSelected(InSpawnPoint);

	// 첫 스폰 완료 시 군집 시스템 가동
	if (bFirstSpawnCompleted)
	{
		UTPSSwarmSubsystem* pSwarmSub = GetWorld()->GetSubsystem<UTPSSwarmSubsystem>();
		if (ensure(pSwarmSub))
		{
			pSwarmSub->StartWaveSystem();
		}
	}
}
