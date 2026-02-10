#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Moveable.generated.h"

UINTERFACE()
class UMoveable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TPS_API IMoveable
{
	GENERATED_BODY()

public:
	virtual void Move(const FVector2D& InputVector) = 0;
};