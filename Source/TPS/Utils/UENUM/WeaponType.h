#pragma once

#include "CoreMinimal.h"
#include "WeaponType.generated.h"

/**
 * 무기 종류
 * - TPSWeaponBase에서 무기 분류에 사용
 * - AnimLayer 전환, DataTable 조회 키로 활용
 */
UENUM()
enum class EWeaponType : uint8
{
	Pistol,  /** 권총 */
	Rifle,   /** 소총 (어썰트 라이플) */
	Shotgun  /** 산탄총 */
};
