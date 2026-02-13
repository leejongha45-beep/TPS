#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TPSLinkedAnimInstance.generated.h"

class UTPSPlayerCoreAnimInstance;

UCLASS()
class TPS_API UTPSLinkedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

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
	UTPSPlayerCoreAnimInstance* GetCoreAnimInstance() const;
};
