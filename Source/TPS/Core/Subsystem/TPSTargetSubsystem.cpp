#include "TPSTargetSubsystem.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTPSTarget, Log, All);
DEFINE_LOG_CATEGORY(LogTPSTarget);

void UTPSTargetSubsystem::RegisterTargetableActor(const TScriptInterface<ITargetable>& TargetableActor)
{
	if (!ensure(TargetableActor.GetObject())) return;
	TargetableActors.AddUnique(TargetableActor);
	UE_LOG(LogTPSTarget, Verbose, TEXT("RegisterTargetableActor: %s (Count: %d)"),
		*TargetableActor.GetObject()->GetName(), TargetableActors.Num());
}

void UTPSTargetSubsystem::UnregisterTargetableActor(const TScriptInterface<ITargetable>& TargetableActor)
{
	TargetableActors.Remove(TargetableActor);
	UE_LOG(LogTPSTarget, Verbose, TEXT("UnregisterTargetableActor: %s (Count: %d)"),
		TargetableActor.GetObject() ? *TargetableActor.GetObject()->GetName() : TEXT("nullptr"), TargetableActors.Num());
}

void UTPSTargetSubsystem::SetAllyBaseLocation(const FVector& InLocation)
{
	AllyBaseLocation = InLocation;
	UE_LOG(LogTPSTarget, Log, TEXT("SetAllyBaseLocation: %s"), *InLocation.ToString());
}
