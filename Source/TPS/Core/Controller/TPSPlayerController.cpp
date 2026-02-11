#include "Core/Controller/TPSPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Utils/Interface/Action/Moveable.h"

ATPSPlayerController::ATPSPlayerController()
{
	SetShowMouseCursor(false);
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
			pEnhancedInput->BindAction(MoveActionAsset, ETriggerEvent::Started, this, &ATPSPlayerController::StartMoveInput);
			pEnhancedInput->BindAction(MoveActionAsset, ETriggerEvent::Triggered, this, &ATPSPlayerController::MoveInput);
			pEnhancedInput->BindAction(MoveActionAsset, ETriggerEvent::Completed, this, &ATPSPlayerController::StopMoveInput);
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

void ATPSPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	SetInputMode(FInputModeGameOnly());
}

void ATPSPlayerController::Look(const FInputActionValue& InputValue)
{
	const FVector2D LookValue = InputValue.Get<FVector2D>();

	AddYawInput(LookValue.X);
	AddPitchInput(LookValue.Y);
}

void ATPSPlayerController::StartMoveInput(const FInputActionValue& InputValue)
{
	if (ensure(MoveableInterface))
	{
		MoveableInterface->StartMove();
	}
}

void ATPSPlayerController::MoveInput(const FInputActionValue& InputValue)
{
	const FVector2D InputVector = InputValue.Get<FVector2D>();

	if (ensure(MoveableInterface))
	{
		MoveableInterface->Move(InputVector);
	}
}

void ATPSPlayerController::StopMoveInput(const FInputActionValue& InputValue)
{
	if (ensure(MoveableInterface))
	{
		MoveableInterface->StopMove();
	}
}