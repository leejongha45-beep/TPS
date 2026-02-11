#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Sprintable.generated.h"

UINTERFACE()
class USprintable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TPS_API ISprintable
{
	GENERATED_BODY()

public:
	virtual void StartSprint() = 0;
	virtual void StopSprint() = 0;
};