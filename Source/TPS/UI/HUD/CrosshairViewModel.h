#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CrosshairViewModel.generated.h"

USTRUCT(BlueprintType)
struct FCrosshairConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float BaseSpreadOffset = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float MovingSpreadAdditive = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SprintingSpreadAdditive = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float AirborneSpreadAdditive = 25.f;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float AimingSpreadMultiplier = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadInterpSpeed = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	float LineLength = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	float LineThickness = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	float CenterDotSize = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FLinearColor CrosshairColor = FLinearColor::White;
};

UCLASS()
class TPS_API UCrosshairViewModel : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(TObjectPtr<class ATPSPlayer> InPlayer, const FCrosshairConfig& InConfig);
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

	float CalculateTargetSpread() const;
};
