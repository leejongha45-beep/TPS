#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TPSPlayerController.generated.h"

UCLASS()
class TPS_API ATPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATPSPlayerController();

protected:
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;

#pragma region Input
	void Look(const struct FInputActionValue& InputValue);
	void StartMoveInput(const struct FInputActionValue& InputValue);
	void MoveInput(const struct FInputActionValue& InputValue);
	void StopMoveInput(const struct FInputActionValue& InputValue);
	void StartSprintInput(const struct FInputActionValue& InputValue);
	void StopSprintInput(const struct FInputActionValue& InputValue);
	void StartAimInput(const struct FInputActionValue& InputValue);
	void StopAimInput(const struct FInputActionValue& InputValue);
	void StartJumpInput(const struct FInputActionValue& InputValue);
	void StopJumpInput(const struct FInputActionValue& InputValue);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> DefaultMappingContextAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> LookActionAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> MoveActionAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> SprintActionAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> AimActionAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> JumpActionAsset;

	UPROPERTY()
	TScriptInterface<class IMoveable> MoveableInterface;

	UPROPERTY()
	TScriptInterface<class ISprintable> SprintableInterface;

	UPROPERTY()
	TScriptInterface<class IAimable> AimableInterface;

	UPROPERTY()
	TScriptInterface<class IJumpable> JumpableInterface;
#pragma endregion
};