#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TPSVillageHUD.generated.h"

/**
 * 마을 전용 HUD
 * - 무기 장착 UI, 스테이지 선택 UI 관리 예정
 * - 전투 HUD(크로스헤어, 탄약 등) 없음
 */
UCLASS()
class TPS_API ATPSVillageHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
