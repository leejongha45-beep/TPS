#include "FAIControlTickFunction.h"
#include "Utils/Interface/Action/AIControllable.h"

void FAIControlTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	if (Target)
	{
		Target->AI_Control_Tick(DeltaTime);
	}
}

FString FAIControlTickFunction::DiagnosticMessage()
{
	return TEXT("FAIControlTickFunction");
}
