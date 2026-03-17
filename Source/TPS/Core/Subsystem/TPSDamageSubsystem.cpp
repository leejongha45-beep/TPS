#include "TPSDamageSubsystem.h"

void UTPSDamageSubsystem::RegisterDamageableActor(const TScriptInterface<IDamageable>& DamageableActor)
{
	if (!ensure(DamageableActor.GetObject())) return;
	DamageableActors.AddUnique(DamageableActor);
}

void UTPSDamageSubsystem::UnregisterDamageableActor(const TScriptInterface<IDamageable>& DamageableActor)
{
	DamageableActors.Remove(DamageableActor);
}
