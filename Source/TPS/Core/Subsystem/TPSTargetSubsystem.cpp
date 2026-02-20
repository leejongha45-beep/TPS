#include "TPSTargetSubsystem.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTPSTarget, Log, All);
DEFINE_LOG_CATEGORY(LogTPSTarget);

void UTPSTargetSubsystem::RegisterTargetableActor(AActor* TargetableActor)
{
	if (!ensure(TargetableActor)) return;
	TargetableActors.AddUnique(TargetableActor);
	UE_LOG(LogTPSTarget, Verbose, TEXT("RegisterTargetableActor: %s (Count: %d)"),
		*TargetableActor->GetName(), TargetableActors.Num());
}

void UTPSTargetSubsystem::UnregisterTargetableActor(AActor* TargetableActor)
{
	TargetableActors.Remove(TargetableActor);
	UE_LOG(LogTPSTarget, Verbose, TEXT("UnregisterTargetableActor: %s (Count: %d)"),
		TargetableActor ? *TargetableActor->GetName() : TEXT("nullptr"), TargetableActors.Num());
}

void UTPSTargetSubsystem::SetAllyBaseLocation(const FVector& InLocation)
{
	AllyBaseLocation = InLocation;
	UE_LOG(LogTPSTarget, Log, TEXT("SetAllyBaseLocation: %s"), *InLocation.ToString());
}
