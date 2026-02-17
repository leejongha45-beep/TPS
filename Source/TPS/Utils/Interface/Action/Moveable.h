#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Moveable.generated.h"

UINTERFACE()
class UMoveable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 이동 액션 인터페이스
 * - Controller → Player(Character) 이동 명령 전달
 * - TScriptInterface<IMoveable>로 Controller가 보유
 */
class TPS_API IMoveable
{
	GENERATED_BODY()

public:
	/** 이동 시작 시 호출 (키 입력 시작) */
	virtual void StartMove() = 0;

	/** 매 프레임 이동 방향 전달 (WASD → FVector2D) */
	virtual void Move(const FVector2D& InputVector) = 0;

	/** 이동 종료 시 호출 (키 릴리스) */
	virtual void StopMove() = 0;
};