#pragma once
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

/**
 * AI 제어 전용 커스텀 Tick 함수
 * - IAIControllable::AI_Control_Tick(DeltaTime)을 매 프레임 호출
 * - Activate/Deactivate로 Enable/Disable 제어
 * - FInterpolateTickFunction과 동일 패턴
 */
struct FAIControlTickFunction : public FTickFunction
{
	/** 호출 대상 (IAIControllable 구현체) */
	class IAIControllable* Target = nullptr;

	/** Tick 실행 — Target->AI_Control_Tick(DeltaTime) 호출 */
	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
	virtual FString DiagnosticMessage() override;
};

template<>
struct TStructOpsTypeTraits<FAIControlTickFunction> : public TStructOpsTypeTraitsBase2<FAIControlTickFunction>
{
	enum { WithCopy = false };
};
