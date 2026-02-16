#pragma once

#include "CoreMinimal.h"
#include "State.generated.h"

UENUM(Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EActionState : uint16
{
	None    = 0       UMETA(Hidden),
	Idle    = 1 << 0, // 1
	Moving  = 1 << 1, // 2
	Jumping = 1 << 2, // 4
	Falling   = 1 << 3, // 8
	Sprinting = 1 << 4, // 16
	Aiming    = 1 << 5, // 32
	Equipping   = 1 << 6, // 64
	Interacting = 1 << 7, // 128
	Firing      = 1 << 8, // 256
};
ENUM_CLASS_FLAGS(EActionState);
