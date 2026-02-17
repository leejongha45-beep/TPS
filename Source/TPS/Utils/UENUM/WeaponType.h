#pragma once

#include "CoreMinimal.h"
#include "WeaponType.generated.h"

UENUM()
enum class EWeaponType : uint8
{
	Pistol,
	Rifle,
	Shotgun
};
