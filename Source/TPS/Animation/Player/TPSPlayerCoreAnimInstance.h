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
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	TWeakObjectPtr<class ATPSPlayer> OwnerRef;

	UPROPERTY(BlueprintReadOnly, Category="Status")
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Status")
	float Direction = 0.f;
};