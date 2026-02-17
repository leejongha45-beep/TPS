#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Jumpable.generated.h"

UINTERFACE()
class UJumpable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 점프 액션 인터페이스
 * - Controller → Player(Character) 점프 명령 전달
 * - EActionState::Jumping / Falling 플래그와 연동
 */
class TPS_API IJumpable
{
	GENERATED_BODY()

public:
	/** 점프 시작 (Space 입력) */
	virtual void StartJump() = 0;

	/** 점프 입력 종료 (Space 릴리스) */
	virtual void StopJump() = 0;
};
