#include "TPSTargetSubsystem.h"
#include "Actor/AllyBase/TPSAllyBase.h"
#include "Actor/EnemyBase/TPSEnemyBase.h"

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

void UTPSTargetSubsystem::RegisterAllyBase(ATPSAllyBase* InBase)
{
	if (!ensure(InBase)) return;
	AllyBases.AddUnique(InBase);
	UE_LOG(LogTPSTarget, Log, TEXT("RegisterAllyBase: %s (Count: %d)"), *InBase->GetName(), AllyBases.Num());
}

void UTPSTargetSubsystem::UnregisterAllyBase(ATPSAllyBase* InBase)
{
	AllyBases.Remove(InBase);
	UE_LOG(LogTPSTarget, Log, TEXT("UnregisterAllyBase: %s (Count: %d)"),
		InBase ? *InBase->GetName() : TEXT("nullptr"), AllyBases.Num());
}

void UTPSTargetSubsystem::RegisterEnemyBase(ATPSEnemyBase* InBase)
{
	if (!ensure(InBase)) return;
	EnemyBases.AddUnique(InBase);
	UE_LOG(LogTPSTarget, Log, TEXT("RegisterEnemyBase: %s (Count: %d)"), *InBase->GetName(), EnemyBases.Num());
}

void UTPSTargetSubsystem::UnregisterEnemyBase(ATPSEnemyBase* InBase)
{
	EnemyBases.Remove(InBase);
	UE_LOG(LogTPSTarget, Log, TEXT("UnregisterEnemyBase: %s (Count: %d)"),
		InBase ? *InBase->GetName() : TEXT("nullptr"), EnemyBases.Num());
}

void UTPSTargetSubsystem::RegisterNPC(AActor* InNPC)
{
	if (!ensure(InNPC)) return;
	NPCs.AddUnique(InNPC);
	UE_LOG(LogTPSTarget, Log, TEXT("RegisterNPC: %s (Count: %d)"), *InNPC->GetName(), NPCs.Num());
}

void UTPSTargetSubsystem::UnregisterNPC(AActor* InNPC)
{
	NPCs.Remove(InNPC);
	UE_LOG(LogTPSTarget, Log, TEXT("UnregisterNPC: %s (Count: %d)"),
		InNPC ? *InNPC->GetName() : TEXT("nullptr"), NPCs.Num());
}

void UTPSTargetSubsystem::SetAllyBaseLocation(const FVector& InLocation)
{
	AllyBaseLocation = InLocation;
	UE_LOG(LogTPSTarget, Log, TEXT("SetAllyBaseLocation: %s"), *InLocation.ToString());
}
