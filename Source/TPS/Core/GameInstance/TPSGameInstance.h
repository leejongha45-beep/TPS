#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TPSGameInstance.generated.h"

/**
 * 게임 인스턴스
 * - 게임 전체 생명주기 동안 유지되는 싱글톤
 * - 전역 초기화 로직 (Init)
 */
UCLASS()
class TPS_API UTPSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
};
