#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Utils/UENUM/RootYawOffsetMode.h"
#include "TPSPlayerCoreAnimInstance.generated.h"

/**
 * 코어 애니메이션 인스턴스 (Lyra 스타일 멀티스레드)
 * - NativeUpdateAnimation (게임 스레드): UObject 접근, StateComponent 읽기, LineTrace
 * - NativeThreadSafeUpdateAnimation (워커 스레드): 캐시된 값 타입으로 순수 수학 연산
 * - Turn-In-Place, 조준 Pitch/Yaw, 이동 방향 등 계산
 * - BlueprintThreadSafe Getter로 AnimGraph에서 안전하게 접근
 */
UCLASS()
class TPS_API UTPSPlayerCoreAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	/** 소유 캐릭터 (Player/NPC 공용, WeakPtr) */
	TWeakObjectPtr<class ATPSSoldierBase> OwnerRef;

	/** 상태 컴포넌트 참조 (WeakPtr) */
	TWeakObjectPtr<class UTPSPlayerStateComponent> StateComponentRef;

	/** 워커 스레드용 캐시 (게임 스레드에서 수집, 워커 스레드에서 읽기) */
	FVector CachedVelocity = FVector::ZeroVector;
	FRotator CachedActorRotation = FRotator::ZeroRotator;
	FRotator CachedAimRotation = FRotator::ZeroRotator;

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

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	uint8 bIsFiring : 1 = false;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	uint8 bIsReloading : 1 = false;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float GroundDistance = 0.f;

	// === Turn in Place ===
	UPROPERTY(BlueprintReadOnly, Category = "TurnInPlace")
	float RootYawOffset = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "TurnInPlace")
	ERootYawOffsetMode RootYawOffsetMode = ERootYawOffsetMode::BlendOut;

	UPROPERTY(BlueprintReadOnly, Category = "TurnInPlace")
	float AimYawRate = 0.f;

	// === Locomotion ===
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	uint8 bHasAcceleration : 1 = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float LocalVelocityDirectionAngle = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	uint8 bIsMovingForward : 1 = false;

	// === Upper Body ===
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float UpperBodyBlendWeight = 0.f;

	/** 이전 프레임 캐시 (게임 스레드 전용) */
	float PreviousControllerYaw = 0.f;

#pragma region EquipMontage
	/** 장착 몽타주 에셋 */
	UPROPERTY(EditDefaultsOnly, Category = "Montage|Equip")
	TObjectPtr<UAnimMontage> EquipMontageAsset;

	/** 해제 몽타주 에셋 */
	UPROPERTY(EditDefaultsOnly, Category = "Montage|Equip")
	TObjectPtr<UAnimMontage> UnequipMontageAsset;

	/** 장착 컴포넌트 참조 (WeakPtr) */
	TWeakObjectPtr<class UTPSEquipComponent> EquipComponentRef;

	/** 장착/해제 몽타주 재생 (bEquip: true=장착, false=해제) */
	void PlayEquipMontage(bool bEquip);

	/** 몽타주 종료 콜백 — EquipComponent에 결과 전달 */
	UFUNCTION()
	void OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted);
#pragma endregion

#pragma region FireMontage
	/** 발사 몽타주 에셋 */
	UPROPERTY(EditDefaultsOnly, Category = "Montage|Fire")
	TObjectPtr<UAnimMontage> FireMontageAsset;

	/** FireComponent 참조 (WeakPtr) */
	TWeakObjectPtr<class UTPSFireComponent> FireComponentRef;

	/** 발사 상태 변경 시 몽타주 정지 (FireComponent 델리게이트에서 호출) */
	void OnFireStateChanged(bool bFiring);

	/** 발사 1회 시 몽타주 재재생 (FireComponent에서 호출) */
	void OnFireOnce();
#pragma endregion

#pragma region ReloadMontage
	/** 재장전 몽타주 에셋 */
	UPROPERTY(EditDefaultsOnly, Category = "Montage|Reload")
	TObjectPtr<UAnimMontage> ReloadMontageAsset;

	/** 재장전 몽타주 재생 (FireComponent 델리게이트에서 호출) */
	void PlayReloadMontage();

	/** 재장전 몽타주 종료 콜백 — FireComponent에 결과 전달 */
	UFUNCTION()
	void OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted);
#pragma endregion

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
	bool GetIsFiring() const { return bIsFiring; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	bool GetIsReloading() const { return bIsReloading; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetGroundDistance() const { return GroundDistance; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetRootYawOffset() const { return RootYawOffset; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	ERootYawOffsetMode GetRootYawOffsetMode() const { return RootYawOffsetMode; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetAimYawRate() const { return AimYawRate; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	bool GetHasAcceleration() const { return bHasAcceleration; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetLocalVelocityDirectionAngle() const { return LocalVelocityDirectionAngle; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	bool GetIsMovingForward() const { return bIsMovingForward; }

	UFUNCTION(BlueprintPure, Category="Animation", meta=(BlueprintThreadSafe))
	float GetUpperBodyBlendWeight() const { return UpperBodyBlendWeight; }

	/** Linked Layer에서 호출 — 스테이트 전환 시 모드 변경 */
	UFUNCTION(BlueprintCallable, Category="Animation", meta=(BlueprintThreadSafe))
	void SetRootYawOffsetMode(ERootYawOffsetMode InMode);
};