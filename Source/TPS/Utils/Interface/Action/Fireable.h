#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Fireable.generated.h"

UINTERFACE()
class UFireable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 사격 액션 인터페이스
 * - Controller → Player(Character) 사격 명령 전달
 * - EActionState::Firing 플래그와 연동
 * - TPSFireComponent에서 발사체 스폰 + 이펙트 처리
 */
class TPS_API IFireable
{
	GENERATED_BODY()

public:
	/** 사격 시작 (LMB 입력) */
	virtual void StartFire() = 0;

	/** 사격 종료 (LMB 릴리스) */
	virtual void StopFire() = 0;
};
