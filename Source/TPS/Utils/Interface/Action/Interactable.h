#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE()
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

class TPS_API IInteractable
{
	GENERATED_BODY()

public:
	virtual void Interact() = 0;
};
