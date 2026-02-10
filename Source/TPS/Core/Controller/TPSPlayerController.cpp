#include "Core/Controller/TPSPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Utils/Interface/Action/Moveable.h"

ATPSPlayerController::ATPSPlayerController()
{
}

void ATPSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// IMC
	UEnhancedInputLocalPlayerSubsystem* pSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (ensure(pSubsystem))
	{
		if (ensure(DefaultMappingContextAsset))
		{
			pSubsystem->AddMappingContext(DefaultMappingContextAsset, 0);
		}
	}

	// ActionBinding
	UEnhancedInputComponent* pEnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (pEnhancedInput)
	{
		if (ensure(LookActionAsset))
		{
			pEnhancedInput->BindAction(LookActionAsset, ETriggerEvent::Triggered, this, &ATPSPlayerController::Look);
		}

		if (ensure(MoveActionAsset))
		{
			pEnhancedInput->BindAction(MoveActionAsset, ETriggerEvent::Triggered, this, &ATPSPlayerController::MoveInput);
		}
	}
}

void ATPSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ensure(InPawn))
	{
		MoveableInterface = InPawn;
	}
}

void ATPSPlayerController::Look(const FInputActionValue& InputValue)
{
	const FVector2D LookValue = InputValue.Get<FVector2D>();

	AddYawInput(LookValue.X);
	AddPitchInput(LookValue.Y);
}

void ATPSPlayerController::MoveInput(const FInputActionValue& InputValue)
{
	const FVector2D InputVector = InputValue.Get<FVector2D>();

	if (MoveableInterface)
	{
		MoveableInterface->Move(InputVector);
	}
}