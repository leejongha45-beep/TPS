#pragma once

#include "CoreMinimal.h"
#include "Pawn/Character/Base/TPSSoldierBase.h"
#include "Utils/Interface/Data/Interpolable.h"
#include "Utils/TickFunctions/FInterpolateTickFunction.h"
#include "TPSPlayer.generated.h"

/** AmmoViewModel 변경 시 브로드캐스트 (장착: ViewModel 전달, 해제: nullptr) */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAmmoViewModelChanged, class UAmmoViewModel* /* InAmmoViewModel */);

/**
 * 플레이어 캐릭터
 * - ATPSSoldierBase 상속 (이동/스프린트/점프/조준/장착/사격 공통)
 * - Player 전용: 카메라, 상호작용, ADS 보간
 * - Controller에서 인터페이스를 통해 액션 명령 수신
 */
UCLASS()
class TPS_API ATPSPlayer
	: public ATPSSoldierBase, public IInterpolable
{
	GENERATED_BODY()

public:
	ATPSPlayer(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE class UTPSPlayerInteractionComponent* GetInteractionComponent() const { return InteractionComponentInst; }

	FOnAmmoViewModelChanged OnAmmoViewModelChangedDelegate;

protected:
	virtual void PostInitializeComponents() override;
	virtual void RegisterActorTickFunctions(bool bRegister) override;
	virtual void CreateDefaultComponents() override;

#pragma region Component
	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Camera")
	TObjectPtr<class USpringArmComponent> SpringArmComponentInst;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Camera")
	TObjectPtr<class UCameraComponent> CameraComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Camera")
	TObjectPtr<class UTPSCameraControlComponent> CameraControlComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Interaction")
	TObjectPtr<class UTPSPlayerInteractionComponent> InteractionComponentInst;
#pragma endregion

#pragma region PlayerOverride
	virtual void Interact() override;
	virtual void StartAim() override;
	virtual void StopAim() override;
	virtual void StartFire() override;
	virtual void OnEquipStateChanged(bool bIsEquipped) override;
#pragma endregion

#pragma region AimRotation
	virtual void Interpolate_Tick(float DeltaTime) override;
	void SetInterpolateTickEnabled(bool bEnabled);

	FInterpolateTickFunction InterpolateTickFunction;

	/** 조준 시 캐릭터 회전 보간 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Aim")
	float AimRotationInterpSpeed = 10.f;
#pragma endregion
};
