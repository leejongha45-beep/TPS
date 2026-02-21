#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

class TPS_API IDamageable
{
	GENERATED_BODY()

public:
	virtual float ReceiveDamage(float Damage, AActor* DamageCauser) = 0;
	virtual FVector GetDamageableLocation() const = 0;
	virtual bool IsDamageable() const = 0;
};
