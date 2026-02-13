#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Equippable.generated.h"

UINTERFACE()
class UEquippable : public UInterface
{
	GENERATED_BODY()
};

class TPS_API IEquippable
{
	GENERATED_BODY()

public:
	virtual void Equip() = 0;
	virtual void Unequip() = 0;
};
