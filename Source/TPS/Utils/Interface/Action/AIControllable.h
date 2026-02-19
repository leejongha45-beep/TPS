#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AIControllable.generated.h"

UINTERFACE(MinimalAPI)
class UAIControllable : public UInterface
{
	GENERATED_BODY()
};

/**
 * AI 제어 가능 인터페이스
 * - FAIControlTickFunction에서 매 프레임 AI_Control_Tick 호출
 * - 적 폰, 아군 NPC 등 AI Tick이 필요한 모든 대상이 구현
 */
class TPS_API IAIControllable
{
	GENERATED_BODY()

public:
	/** AI 제어 Tick — 상태 전환, 이동, 공격 등 */
	virtual void AI_Control_Tick(float DeltaTime) = 0;
};
