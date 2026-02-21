#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TPSGameModeBase.generated.h"

/**
 * 게임 모드 베이스
 * - 기본 Player, Controller, HUD 클래스 지정
 * - 텔레포트 방식 스폰: 기본 스폰 후 UI 선택 → 텔레포트
 */
UCLASS()
class TPS_API ATPSGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATPSGameModeBase();

	/** 스폰 선택 UI 콜백 — 선택된 위치로 Pawn 텔레포트 */
	void OnSpawnPointSelected(class ATPSPlayerStart* InSpawnPoint);

protected:
	virtual void BeginPlay() override;

#pragma region SpawnSelect
	/** 스폰 선택 UI 표시 (Pawn 숨김 + 입력 차단 + UI 표시) */
	void ShowSpawnSelect();

	/** 첫 스폰 선택 완료 여부 */
	uint8 bFirstSpawnCompleted : 1 = false;
#pragma endregion
};
