#pragma once

#include "CoreMinimal.h"
#include "RootYawOffsetMode.generated.h"

UENUM(BlueprintType)
enum class ERootYawOffsetMode : uint8
{
	BlendOut    UMETA(DisplayName = "Blend Out"),
	Hold        UMETA(DisplayName = "Hold"),
	Accumulate  UMETA(DisplayName = "Accumulate")
};
