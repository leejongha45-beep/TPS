#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interpolable.generated.h"

UINTERFACE()
class UInterpolable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TPS_API IInterpolable
{
	GENERATED_BODY()

public:
	virtual void Interpolate_Tick(float DeltaTime) = 0;
};