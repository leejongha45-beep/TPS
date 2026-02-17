#pragma once

#include "CoreMinimal.h"
#include "Weapon/TPSWeaponBase.h"
#include "TPSRifle.generated.h"

/**
 * 어썰트 라이플
 * - WeaponBase의 기본 스탯 사용
 * - 생성자에서 라이플 전용 메시/스탯 설정
 */
UCLASS()
class TPS_API ATPSRifle : public ATPSWeaponBase
{
	GENERATED_BODY()

public:
	ATPSRifle();
};
