#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE()
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 상호작용 액션 인터페이스
 * - Controller → Player(Character) 상호작용 명령 전달
 * - EActionState::Interacting 플래그와 연동
 * - TPSPlayerInteractionComponent에서 오버랩 대상과 상호작용
 */
class TPS_API IInteractable
{
	GENERATED_BODY()

public:
	/** 상호작용 실행 (E 키 입력) */
	virtual void Interact() = 0;
};
