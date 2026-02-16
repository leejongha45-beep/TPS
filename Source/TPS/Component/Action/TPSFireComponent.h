#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSFireComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSFireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void StartFire();
	void StopFire();

	FORCEINLINE bool GetIsFiring() const { return bIsFiring; }

protected:
	void FireOnce();
	FVector CalculateShotDirection(const FVector& InMuzzleLocation) const;
	void SpawnMuzzleEffect(const FTransform& InMuzzleTransform);
	void ActivateProjectileFromPool(const FTransform& InMuzzleTransform, const FVector& InDirection);

	FTimerHandle FireTimerHandle;

	uint8 bIsFiring : 1 = false;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<class UNiagaraSystem> MuzzleFlashEffectAsset;
};
