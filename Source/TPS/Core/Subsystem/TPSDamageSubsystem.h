#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Utils/Interface/Data/Damageable.h"
#include "TPSDamageSubsystem.generated.h"

UCLASS()
class TPS_API UTPSDamageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterDamageableActor(const TScriptInterface<IDamageable>& DamageableActor);
	void UnregisterDamageableActor(const TScriptInterface<IDamageable>& DamageableActor);

	FORCEINLINE const TArray<TScriptInterface<IDamageable>>& GetDamageableActors() const
	{
		checkSlow(IsInGameThread());
		return DamageableActors;
	}

private:
	TArray<TScriptInterface<IDamageable>> DamageableActors;
};
