#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Utils/Interface/Action/Aimable.h"
#include "Utils/Interface/Action/Equippable.h"
#include "Utils/Interface/Action/Jumpable.h"
#include "Utils/Interface/Action/Moveable.h"
#include "Utils/Interface/Action/Sprintable.h"
#include "Utils/Interface/Action/Interactable.h"
#include "Utils/Interface/Action/Fireable.h"
#include "Utils/Interface/Data/Interpolable.h"
#include "Utils/TickFunctions/FInterpolateTickFunction.h"
#include "TPSPlayer.generated.h"

UCLASS()
class TPS_API ATPSPlayer
	: public ACharacter, public IMoveable, public ISprintable, public IAimable, public IJumpable, public IEquippable, public IInterpolable, public IInteractable, public IFireable
{
	GENERATED_BODY()

public:
	ATPSPlayer(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE class UTPSPlayerStateComponent* GetStateComponent() const { return StateComponentInst; }
	FORCEINLINE class UTPSAnimLayerComponent* GetAnimLayerComponent() const { return AnimLayerComponentInst; }
	FORCEINLINE class UTPSPlayerInteractionComponent* GetInteractionComponent() const { return InteractionComponentInst; }
	FORCEINLINE class UTPSEquipComponent* GetEquipComponent() const { return EquipComponentInst; }
	FORCEINLINE class UTPSFireComponent* GetFireComponent() const { return FireComponentInst; }

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void RegisterActorTickFunctions(bool bRegister) override;
	virtual void OnJumped_Implementation() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	void BindDelegate();
	void CreateDefaultComponents();

#pragma region Component

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Camera")
	TObjectPtr<class USpringArmComponent> SpringArmComponentInst;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Camera")
	TObjectPtr<class UCameraComponent> CameraComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Action")
	TObjectPtr<class UTPSCMC> CMCInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|State")
	TObjectPtr<class UTPSPlayerStateComponent> StateComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Status")
	TObjectPtr<class UTPSPlayerStatusComponent> StatusComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Camera")
	TObjectPtr<class UTPSCameraControlComponent> CameraControlComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Action")
	TObjectPtr<class UTPSEquipComponent> EquipComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Animation")
	TObjectPtr<class UTPSAnimLayerComponent> AnimLayerComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Interaction")
	TObjectPtr<class UTPSPlayerInteractionComponent> InteractionComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Action")
	TObjectPtr<class UTPSFireComponent> FireComponentInst;
#pragma endregion

#pragma region ControllerCallback
	virtual void StartMove() override;
	virtual void Move(const FVector2D& InputVector) override;
	virtual void StopMove() override;

	virtual void StartSprint() override;
	virtual void StopSprint() override;

	virtual void StartAim() override;
	virtual void StopAim() override;

	virtual void StartJump() override;
	virtual void StopJump() override;

	virtual void Equip() override;
	virtual void Unequip() override;

	virtual void Interact() override;

	virtual void StartFire() override;
	virtual void StopFire() override;
#pragma endregion

#pragma region EquipCallback
	void OnEquipStateChanged(bool bIsEquipped);
#pragma endregion

#pragma region FireCallback
	void OnFireStateChanged(bool bIsFiring);
#pragma endregion

#pragma region AimRotation
	virtual void Interpolate_Tick(float DeltaTime) override;
	void SetInterpolateTickEnabled(bool bEnabled);

	FInterpolateTickFunction InterpolateTickFunction;

	UPROPERTY(EditDefaultsOnly, Category = "Aim")
	float AimRotationInterpSpeed = 10.f;
#pragma endregion
};