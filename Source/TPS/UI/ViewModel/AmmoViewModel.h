#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AmmoViewModel.generated.h"

/**
 * 탄약 뷰모델
 * - WeaponBase가 소유, 탄약 변동 시 SetAmmo()로 값 푸시
 * - HUD(View)는 Getter로 읽기만 수행
 */
UCLASS()
class TPS_API UAmmoViewModel : public UObject
{
	GENERATED_BODY()

public:
	/** WeaponBase에서 탄약 변동 시 호출 (푸시) */
	FORCEINLINE void SetAmmo(int32 InCurrent, int32 InMax) { CurrentAmmo = InCurrent; MaxAmmo = InMax; }

	FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }
	FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }

protected:
	int32 CurrentAmmo = 0;
	int32 MaxAmmo = 0;
};
