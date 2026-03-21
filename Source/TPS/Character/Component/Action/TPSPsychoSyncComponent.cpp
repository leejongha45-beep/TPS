#include "TPSPsychoSyncComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Character/Component/Action/TPSFireComponent.h"
#include "ECS/Scheduler/EnemyManagerSubsystem.h"

void UTPSPsychoSyncComponent::Initialize()
{
	UWorld* pWorld = GetWorld();
	if (!pWorld) { return; }

	// ECS 킬 델리게이트 바인딩
	if (UEnemyManagerSubsystem* pEnemyMgr = pWorld->GetSubsystem<UEnemyManagerSubsystem>())
	{
		pEnemyMgr->OnPlayerKillECSDelegate.AddUObject(this, &UTPSPsychoSyncComponent::OnEnemyKilled);
	}
}

void UTPSPsychoSyncComponent::Reset()
{
	KillStreak = 0;
	SetPhase(EPsychoPhase::Phase0);

	if (UWorld* pWorld = GetWorld())
	{
		pWorld->GetTimerManager().ClearTimer(DecayTimerHandle);
	}
}

void UTPSPsychoSyncComponent::OnEnemyKilled(int32 KillCount)
{
	KillStreak += KillCount;

	// 페이즈 업 체크
	EPsychoPhase NewPhase = EPsychoPhase::Phase0;
	if (KillStreak >= Phase2Threshold)
	{
		NewPhase = EPsychoPhase::Phase2_Penetration;
	}
	else if (KillStreak >= Phase1Threshold)
	{
		NewPhase = EPsychoPhase::Phase1_InfiniteAmmo;
	}

	if (NewPhase > CurrentPhase)
	{
		SetPhase(NewPhase);
	}

	// 디케이 타이머 리셋
	ResetDecayTimer();

	UE_LOG(LogTemp, Log, TEXT("[PsychoSync] KillStreak=%d Phase=%d"), KillStreak, static_cast<int32>(CurrentPhase));
}

void UTPSPsychoSyncComponent::OnDecayTimerExpired()
{
	if (CurrentPhase == EPsychoPhase::Phase2_Penetration)
	{
		SetPhase(EPsychoPhase::Phase1_InfiniteAmmo);
		KillStreak = Phase1Threshold;
		ResetDecayTimer();
	}
	else if (CurrentPhase == EPsychoPhase::Phase1_InfiniteAmmo)
	{
		SetPhase(EPsychoPhase::Phase0);
		KillStreak = 0;
		// Phase0이면 타이머 재시작 안 함
	}

	UE_LOG(LogTemp, Log, TEXT("[PsychoSync] Decay — Phase=%d KillStreak=%d"), static_cast<int32>(CurrentPhase), KillStreak);
}

void UTPSPsychoSyncComponent::ResetDecayTimer()
{
	if (UWorld* pWorld = GetWorld())
	{
		pWorld->GetTimerManager().ClearTimer(DecayTimerHandle);

		if (CurrentPhase > EPsychoPhase::Phase0)
		{
			pWorld->GetTimerManager().SetTimer(
				DecayTimerHandle, this, &UTPSPsychoSyncComponent::OnDecayTimerExpired,
				DecayInterval, false);
		}
	}
}

void UTPSPsychoSyncComponent::SetPhase(EPsychoPhase NewPhase)
{
	if (NewPhase == CurrentPhase) { return; }

	const EPsychoPhase OldPhase = CurrentPhase;
	CurrentPhase = NewPhase;

	SyncFireComponent();
	OnPsychoPhaseChangedDelegate.Broadcast(NewPhase, OldPhase);

	UE_LOG(LogTemp, Warning, TEXT("[PsychoSync] Phase changed: %d → %d"),
		static_cast<int32>(OldPhase), static_cast<int32>(NewPhase));
}

void UTPSPsychoSyncComponent::SyncFireComponent()
{
	AActor* pOwner = GetOwner();
	if (!pOwner) { return; }

	UTPSFireComponent* pFire = pOwner->FindComponentByClass<UTPSFireComponent>();
	if (!pFire) { return; }

	pFire->SetInfiniteAmmo(HasInfiniteAmmo());
	pFire->SetPenetration(HasPenetration());
}
