#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

struct FInterpolateTickFunction : public FTickFunction
{
	class IInterpolable* Target = nullptr;

	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
	virtual FString DiagnosticMessage() override;
};

template<>
struct TStructOpsTypeTraits<FInterpolateTickFunction> : public TStructOpsTypeTraitsBase2<FInterpolateTickFunction>
{
	enum { WithCopy = false };
};