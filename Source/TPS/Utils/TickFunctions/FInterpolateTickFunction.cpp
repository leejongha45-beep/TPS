#include "FInterpolateTickFunction.h"
#include "Utils/Interface/Data/Interpolable.h"

void FInterpolateTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	if (Target)
	{
		Target->Interpolate_Tick(DeltaTime);
	}
}

FString FInterpolateTickFunction::DiagnosticMessage()
{
	return TEXT("FInterpolateTickFunction");
}
