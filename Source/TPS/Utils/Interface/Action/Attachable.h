#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Attachable.generated.h"

UINTERFACE()
class UAttachable : public UInterface
{
	GENERATED_BODY()
};

class TPS_API IAttachable
{
	GENERATED_BODY()

public:
	virtual void Attach(USkeletalMeshComponent* InTargetMesh) = 0;
	virtual void Detach(USkeletalMeshComponent* InTargetMesh) = 0;
};
