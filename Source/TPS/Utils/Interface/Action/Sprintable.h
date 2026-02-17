#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Sprintable.generated.h"

UINTERFACE()
class USprintable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 달리기 액션 인터페이스
 * - Controller → Player(Character) 스프린트 명령 전달
 * - EActionState::Sprinting 플래그와 연동
 */
class TPS_API ISprintable
{
	GENERATED_BODY()

public:
	/** 달리기 시작 (Shift 입력) */
	virtual void StartSprint() = 0;

	/** 달리기 종료 (Shift 릴리스) */
	virtual void StopSprint() = 0;
};