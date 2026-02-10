#include "Core/Controller/TPSPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

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
	}
}

void ATPSPlayerController::Look(const FInputActionValue& InputValue)
{
	const FVector2D LookValue = InputValue.Get<FVector2D>();

	AddYawInput(LookValue.X);
	AddPitchInput(LookValue.Y);
}