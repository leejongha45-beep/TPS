#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TPSLinkedAnimInstance.generated.h"

/**
 * ALI(Animation Layer Interface)용 링크드 애님 인스턴스 베이스
 * - CoreAnimInstance에서 ThreadSafe Getter로 값을 가져와 로컬 캐시
 * - NativeThreadSafeUpdateAnimation에서 CoreAnimInstance의 값을 복사
 * - 무기/상태별 BP 서브클래스에서 AnimGraph 구성
 */
UCLASS()
class TPS_API UTPSLinkedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	/** CoreAnimInstance에서 값 복사 (워커 스레드 안전) */
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	float AimPitch = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	float AimYaw = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	uint8 bIsAiming : 1 = false;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	uint8 bIsEquipping : 1 = false;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	uint8 bIsFalling : 1 = false;

	/** 발사 중 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	uint8 bIsFiring : 1 = false;

	/** 재장전 중 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	uint8 bIsReloading : 1 = false;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float GroundDistance = 0.f;

	/** === Turn in Place === */
	UPROPERTY(BlueprintReadOnly, Category = "TurnInPlace")
	float RootYawOffset = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "TurnInPlace")
	float AimYawRate = 0.f;

	/** === Locomotion === */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	uint8 bHasAcceleration : 1 = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float LocalVelocityDirectionAngle = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	uint8 bIsMovingForward : 1 = false;

public:
	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	class UTPSPlayerCoreAnimInstance* GetCoreAnimInstance() const;

	/** Thread-safe getters (SM 트랜지션 룰에서 안전하게 접근) */
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
	bool GetIsFiring() const { return bIsFiring; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	bool GetIsReloading() const { return bIsReloading; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetGroundDistance() const { return GroundDistance; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetRootYawOffset() const { return RootYawOffset; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetAimYawRate() const { return AimYawRate; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	bool GetHasAcceleration() const { return bHasAcceleration; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetLocalVelocityDirectionAngle() const { return LocalVelocityDirectionAngle; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	bool GetIsMovingForward() const { return bIsMovingForward; }
};