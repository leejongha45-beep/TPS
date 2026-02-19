#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CrosshairViewModel.generated.h"

/**
 * 크로스헤어 설정 구조체
 * - 상태별 스프레드 가산값 (이동, 달리기, 공중, 사격)
 * - 조준 시 스프레드 곱수
 * - 라인 스타일 (길이, 두께, 색상)
 */
USTRUCT(BlueprintType)
struct FCrosshairConfig
{
	GENERATED_BODY()

	/** 기본 스프레드 오프셋 (정지 시) */
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float BaseSpreadOffset = 10.f;

	/** 이동 시 스프레드 가산 */
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float MovingSpreadAdditive = 15.f;

	/** 달리기 시 스프레드 가산 */
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SprintingSpreadAdditive = 30.f;

	/** 공중 시 스프레드 가산 */
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float AirborneSpreadAdditive = 25.f;

	/** 사격 시 스프레드 가산 */
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float FiringSpreadAdditive = 5.f;

	/** 조준 시 스프레드 곱수 (0.3 = 70% 축소) */
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float AimingSpreadMultiplier = 0.3f;

	/** 스프레드 보간 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadInterpSpeed = 8.f;

	/** 크로스헤어 라인 길이 */
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	float LineLength = 10.f;

	/** 크로스헤어 라인 두께 */
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	float LineThickness = 2.f;

	/** 중앙 도트 크기 */
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	float CenterDotSize = 2.f;

	/** 크로스헤어 색상 */
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FLinearColor CrosshairColor = FLinearColor::White;
};

/**
 * 크로스헤어 뷰모델
 * - 플레이어 상태에 따라 스프레드 목표값 계산
 * - 매 프레임 현재 스프레드를 목표로 보간
 * - HUD에서 GetSpreadOffset()으로 읽어 드로잉
 */
UCLASS()
class TPS_API UCrosshairViewModel : public UObject
{
	GENERATED_BODY()

public:
	/** 플레이어 참조 + 설정 초기화 */
	void Initialize(TObjectPtr<class ATPSPlayer> InPlayer, const FCrosshairConfig& InConfig);

	/** 매 프레임 스프레드 보간 (HUD::DrawHUD에서 호출) */
	void Update(float DeltaTime);

	FORCEINLINE float GetSpreadOffset() const { return CurrentSpreadOffset; }
	FORCEINLINE float GetLineLength() const { return Config.LineLength; }
	FORCEINLINE float GetLineThickness() const { return Config.LineThickness; }
	FORCEINLINE float GetCenterDotSize() const { return Config.CenterDotSize; }
	FORCEINLINE FLinearColor GetCrosshairColor() const { return Config.CrosshairColor; }
	FORCEINLINE bool GetIsVisible() const { return bIsVisible; }
	FORCEINLINE bool IsPlayerValid() const { return PlayerRef.IsValid(); }

protected:
	TWeakObjectPtr<class ATPSPlayer> PlayerRef;
	TWeakObjectPtr<class UTPSPlayerStateComponent> StateComponentRef;

	FCrosshairConfig Config;

	float TargetSpreadOffset = 0.f;
	float CurrentSpreadOffset = 0.f;
	uint8 bIsVisible : 1 = true;

	/** 상태별 목표 스프레드 계산 (Moving + Sprinting + Airborne + Firing + Aiming) */
	float CalculateTargetSpread() const;
};
