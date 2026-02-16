#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSFireComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFireStateChanged, bool /* bIsFiring */);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TPS_API UTPSFireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void StartFire(class ATPSWeaponBase* InWeapon, TFunction<void (FVector&, FRotator&)> InViewPointGetter);
	void StopFire();

	FORCEINLINE bool GetIsFiring() const { return bIsFiring; }

	FOnFireStateChanged OnFireStateChangedDelegate;

protected:
	void FireOnce();
	FVector CalculateShotDirection(const FVector& InMuzzleLocation) const;
	void SpawnMuzzleEffect(const FTransform& InMuzzleTransform);
	void ActivateProjectileFromPool(const FTransform& InMuzzleTransform, const FVector& InDirection, class ATPSWeaponBase* InWeapon);

	TWeakObjectPtr<class ATPSWeaponBase> WeaponRef;

	FTimerHandle FireTimerHandle;

	uint8 bIsFiring : 1 = false;

	TFunction<void (FVector&, FRotator&)> ViewPointGetter;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<class UNiagaraSystem> MuzzleFlashEffectAsset;
};