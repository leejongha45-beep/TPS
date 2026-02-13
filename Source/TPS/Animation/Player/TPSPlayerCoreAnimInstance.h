#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TPSPlayerCoreAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TPS_API UTPSPlayerCoreAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	TWeakObjectPtr<class ATPSPlayer> OwnerRef;
	TWeakObjectPtr<class UTPSPlayerStateComponent> StateComponentRef;

	// 워커 스레드용 캐시 (게임 스레드에서 수집, 워커 스레드에서 읽기)
	FVector CachedVelocity = FVector::ZeroVector;
	FRotator CachedActorRotation = FRotator::ZeroRotator;
	FRotator CachedAimRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category="Status")
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Status")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Aim")
	float AimPitch = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Aim")
	float AimYaw = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Aim")
	uint8 bIsAiming : 1 = false;

	UPROPERTY(BlueprintReadOnly, Category="Status")
	uint8 bIsEquipping : 1 = false;

	UPROPERTY(BlueprintReadOnly, Category="Status")
	uint8 bIsFalling : 1 = false;

	UPROPERTY(BlueprintReadOnly, Category="Status")
	float GroundDistance = 0.f;

public:
	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetGroundSpeed() const { return GroundSpeed; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetDirection() const { return Direction; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetAimPitch() const { return AimPitch; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetAimYaw() const { return AimYaw; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	bool GetIsAiming() const { return bIsAiming; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	bool GetIsEquipping() const { return bIsEquipping; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	bool GetIsFalling() const { return bIsFalling; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetGroundDistance() const { return GroundDistance; }
};