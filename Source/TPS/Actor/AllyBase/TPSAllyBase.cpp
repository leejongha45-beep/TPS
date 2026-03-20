#include "Actor/AllyBase/TPSAllyBase.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"
#include "Core/Subsystem/TPSDamageSubsystem.h"
#include "Engine/World.h"

ATPSAllyBase::ATPSAllyBase()
{
}

void ATPSAllyBase::BeginPlay()
{
	Super::BeginPlay();

	if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
	{
		TargetSS->RegisterTargetableActor(TScriptInterface<ITargetable>(this));
		TargetSS->RegisterAllyBase(this);
		TargetSS->SetAllyBaseLocation(GetActorLocation());
	}

	if (UTPSDamageSubsystem* DamageSS = GetWorld()->GetSubsystem<UTPSDamageSubsystem>())
	{
		DamageSS->RegisterDamageableActor(TScriptInterface<IDamageable>(this));
	}

	UE_LOG(LogTemp, Log, TEXT("[AllyBase] %s initialized — HP: %.0f / %.0f"),
		*GetName(), GetHealth(), GetMaxHealth());
}

void ATPSAllyBase::OnBaseDestroyed()
{
	if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
	{
		TargetSS->UnregisterTargetableActor(TScriptInterface<ITargetable>(this));
		TargetSS->UnregisterAllyBase(this);
	}
	if (UTPSDamageSubsystem* DamageSS = GetWorld()->GetSubsystem<UTPSDamageSubsystem>())
	{
		DamageSS->UnregisterDamageableActor(TScriptInterface<IDamageable>(this));
	}

	Super::OnBaseDestroyed();
}
