#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Utils/Interface/Action/Aimable.h"
#include "Utils/Interface/Action/Jumpable.h"
#include "Utils/Interface/Action/Moveable.h"
#include "Utils/Interface/Action/Sprintable.h"
#include "Utils/Interface/Data/Interpolable.h"
#include "Utils/TickFunctions/FInterpolateTickFunction.h"
#include "TPSPlayer.generated.h"

UCLASS()
class TPS_API ATPSPlayer : public ACharacter, public IMoveable, public ISprintable, public IAimable, public IJumpable, public IInterpolable
{
	GENERATED_BODY()

public:
	ATPSPlayer(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE class UTPSPlayerStateComponent* GetStateComponent() const { return StateComponentInst; }

	UFUNCTION(BlueprintCallable, Category="Animation")
	void LinkAnimLayer(TSubclassOf<UAnimInstance> InClass);

	UFUNCTION(BlueprintCallable, Category="Animation")
	void UnlinkAnimLayer();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void RegisterActorTickFunctions(bool bRegister) override;
	virtual void OnJumped_Implementation() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	void CreateDefaultComponents();

#pragma region Component

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Camera")
	TObjectPtr<class USpringArmComponent> SpringArmComponentInst;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component|Camera")
	TObjectPtr<class UCameraComponent> CameraComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Action")
	TObjectPtr<class UTPSCMC> CachedCMC;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|State")
	TObjectPtr<class UTPSPlayerStateComponent> StateComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Status")
	TObjectPtr<class UTPSPlayerStatusComponent> StatusComponentInst;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Component|Camera")
	TObjectPtr<class UTPSCameraControlComponent> CameraControlComponentInst;
#pragma endregion

#pragma region AnimLayer
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> DefaultAnimLayerClass;

	UPROPERTY(Transient)
	TSubclassOf<UAnimInstance> CurrentAnimLayerClass;
#pragma endregion

	virtual void StartMove() override;
	virtual void Move(const FVector2D& InputVector) override;
	virtual void StopMove() override;

	virtual void StartSprint() override;
	virtual void StopSprint() override;

	virtual void StartAim() override;
	virtual void StopAim() override;

	virtual void StartJump() override;
	virtual void StopJump() override;

#pragma region AimRotation
	virtual void Interpolate_Tick(float DeltaTime) override;
	void SetInterpolateTickEnabled(bool bEnabled);

	FInterpolateTickFunction InterpolateTickFunction;

	UPROPERTY(EditDefaultsOnly, Category = "Aim")
	float AimRotationInterpSpeed = 10.f;
#pragma endregion
};