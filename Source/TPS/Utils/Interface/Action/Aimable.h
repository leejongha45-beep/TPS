#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Aimable.generated.h"

UINTERFACE()
class UAimable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 조준 액션 인터페이스
 * - Controller → Player(Character) 조준 명령 전달
 * - EActionState::Aiming 플래그와 연동
 * - 카메라 FOV, 조준선 UI 변경 트리거
 */
class TPS_API IAimable
{
	GENERATED_BODY()

public:
	/** 조준 시작 (RMB 입력) */
	virtual void StartAim() = 0;

	/** 조준 종료 (RMB 릴리스) */
	virtual void StopAim() = 0;
};
