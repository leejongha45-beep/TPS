#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Jumpable.generated.h"

UINTERFACE()
class UJumpable : public UInterface
{
	GENERATED_BODY()
};

class TPS_API IJumpable
{
	GENERATED_BODY()

public:
	virtual void StartJump() = 0;
	virtual void StopJump() = 0;
};
