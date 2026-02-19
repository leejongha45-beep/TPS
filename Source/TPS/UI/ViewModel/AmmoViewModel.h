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
	void SetAmmo(int32 InCurrent, int32 InMax);

	FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }
	FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }
	FORCEINLINE FSlateColor GetAmmoColor() const { return AmmoColor; }

protected:
	/** 현재 탄약에 따른 색상 계산 */
	void UpdateAmmoColor();

	int32 CurrentAmmo = 0;
	int32 MaxAmmo = 0;

	/** 탄약 색상 — 20발 이상: 파란, 미만: 빨간 */
	FSlateColor AmmoColor;

	/** 색상 분기 기준 탄약 수 */
	static constexpr int32 LowAmmoThreshold = 20;
};
