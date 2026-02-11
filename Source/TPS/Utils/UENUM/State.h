#pragma once

#include "CoreMinimal.h"
#include "State.generated.h"

UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EActionState : uint8
{
	None    = 0       UMETA(Hidden),
	Idle    = 1 << 0, // 1
	Moving  = 1 << 1, // 2
	Jumping = 1 << 2, // 4
	Falling   = 1 << 3, // 8
	Sprinting = 1 << 4, // 16
};
ENUM_CLASS_FLAGS(EActionState);
