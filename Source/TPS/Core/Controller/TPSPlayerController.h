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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> DefaultMappingContextAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> LookActionAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> MoveActionAsset;

	UPROPERTY()
	TScriptInterface<class IMoveable> MoveableInterface;
#pragma endregion
};