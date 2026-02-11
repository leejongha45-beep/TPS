#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Aimable.generated.h"

UINTERFACE()
class UAimable : public UInterface
{
	GENERATED_BODY()
};

class TPS_API IAimable
{
	GENERATED_BODY()

public:
	virtual void StartAim() = 0;
	virtual void StopAim() = 0;
};
