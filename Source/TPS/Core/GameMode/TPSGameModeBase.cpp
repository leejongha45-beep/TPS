#include "Core/GameMode/TPSGameModeBase.h"

#include "Core/Controller/TPSPlayerController.h"
#include "Pawn/Character/Player/TPSPlayer.h"
#include "UI/HUD/TPSHUD.h"
#include "Wave/TPSWaveManager.h"
#include "Wave/TPSWaveConfig.h"
#include "Enemy/Mass/Processor/TPSEnemyDeathProcessor.h"
#include "Spawn/TPSPlayerStart.h"
#include "Spawn/TPSPlayerSpawnSubsystem.h"

ATPSGameModeBase::ATPSGameModeBase()
{
}

void ATPSGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// ① 웨이브 매니저 초기화만 (StartWaves는 스폰 선택 완료 후)
	if (ensure(WaveConfig))
	{
		WaveManagerInst = NewObject<UTPSWaveManager>(this);
		if (ensure(WaveManagerInst))
		{
			WaveManagerInst->Initialize(GetWorld(), WaveConfig);

			// 킬 델리게이트 바인딩 — DeathProcessor → WaveManager
			UTPSEnemyDeathProcessor::OnEnemyKilledDelegate.AddUObject(
				WaveManagerInst, &UTPSWaveManager::NotifyEnemyKilled);
		}
	}

	// ② 스폰 선택 UI 표시
	ShowSpawnSelect();
}

// ──────────────────────────────────────────────
//  스폰 선택 플로우
// ──────────────────────────────────────────────

void ATPSGameModeBase::ShowSpawnSelect()
{
	APlayerController* pPC = GetWorld()->GetFirstPlayerController();
	if (!ensure(pPC)) return;

	// Pawn 숨김 + 입력 차단
	APawn* pPawn = pPC->GetPawn();
	if (pPawn)
	{
		pPawn->SetActorHiddenInGame(true);
		pPawn->SetActorEnableCollision(false);
		pPC->DisableInput(pPC);
	}

	// 마우스 커서 표시 + UI 인풋 모드
	pPC->SetShowMouseCursor(true);
	pPC->SetInputMode(FInputModeGameAndUI());

	// HUD에서 스폰 선택 위젯 표시
	ATPSHUD* pHUD = Cast<ATPSHUD>(pPC->GetHUD());
	if (ensure(pHUD))
	{
		pHUD->ShowSpawnSelect(this);
	}
}

void ATPSGameModeBase::OnSpawnPointSelected(ATPSPlayerStart* InSpawnPoint)
{
	if (!ensure(InSpawnPoint)) return;

	APlayerController* pPC = GetWorld()->GetFirstPlayerController();
	if (!ensure(pPC)) return;

	APawn* pPawn = pPC->GetPawn();
	if (ensure(pPawn))
	{
		// 선택된 위치로 텔레포트
		pPawn->SetActorLocation(InSpawnPoint->GetActorLocation());
		pPawn->SetActorHiddenInGame(false);
		pPawn->SetActorEnableCollision(true);
		pPC->EnableInput(pPC);
	}

	// 마우스 커서 숨김 + 게임 인풋 모드
	pPC->SetShowMouseCursor(false);
	pPC->SetInputMode(FInputModeGameOnly());

	// HUD에서 스폰 선택 위젯 숨김
	ATPSHUD* pHUD = Cast<ATPSHUD>(pPC->GetHUD());
	if (ensure(pHUD))
	{
		pHUD->HideSpawnSelect();
	}

	// 첫 스폰 완료 시 웨이브 시작
	if (!bFirstSpawnCompleted)
	{
		bFirstSpawnCompleted = true;

		if (ensure(WaveManagerInst))
		{
			WaveManagerInst->StartWaves();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Player spawned at %s (%s)"),
		*InSpawnPoint->GetActorLocation().ToString(),
		*InSpawnPoint->GetDisplayName().ToString());
}
