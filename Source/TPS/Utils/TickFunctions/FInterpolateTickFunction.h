#pragma once

#include "CoreMinimal.h"
#include "FAIControlTickFunction.h"
#include "Engine/EngineBaseTypes.h"

/**
 * 보간 전용 커스텀 Tick 함수
 * - IInterpolable 구현체의 Interpolate_Tick()을 매 프레임 호출
 * - 컴포넌트 Tick과 분리된 독립 Tick으로 보간 전용 제어 가능
 * - RegisterComponentTickFunctions에서 등록, Enable/Disable 제어
 * - UTPSCMC, UTPSCameraControlComponent, ATPSPlayer에서 사용
 */
struct FInterpolateTickFunction : public FTickFunction
{
	/** 호출 대상 (IInterpolable 구현체) */
	class IInterpolable* Target = nullptr;

	/** Tick 실행 — Target->Interpolate_Tick(DeltaTime) 호출 */
	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
	virtual FString DiagnosticMessage() override;
};

template<>
struct TStructOpsTypeTraits<FInterpolateTickFunction> : public TStructOpsTypeTraitsBase2<FInterpolateTickFunction>
{
	enum { WithCopy = false };
};