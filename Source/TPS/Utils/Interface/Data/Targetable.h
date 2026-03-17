#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Targetable.generated.h"

UINTERFACE()
class UTargetable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 적 AI 타겟팅 인터페이스
 * - 적이 공격할 수 있는 모든 대상이 구현 (기지, 플레이어, NPC)
 * - PlayerLocationProcessor가 ITargetable 구현 액터를 수집하여 위치 배열 갱신
 * - MeleeProcessor가 공격 대상 판정 시 IsTargetable() 체크
 */
class TPS_API ITargetable
{
	GENERATED_BODY()

public:
	/** 타겟 월드 위치 반환 */
	virtual FVector GetTargetLocation() const = 0;

	/** 타겟 가능 여부 (생존/활성 상태) */
	virtual bool IsTargetable() const = 0;
};
