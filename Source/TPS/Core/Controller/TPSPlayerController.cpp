#include "Core/Controller/TPSPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Utils/Interface/Action/Aimable.h"
#include "Utils/Interface/Action/Moveable.h"
#include "Utils/Interface/Action/Sprintable.h"

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
	UEnhancedInputComponent* pEnhancedInput = CastChecked<UEnhancedInputComponent>(InputComponent);
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

		if (ensure(SprintActionAsset))
		{
			pEnhancedInput->BindAction(SprintActionAsset, ETriggerEvent::Started, this, &ATPSPlayerController::StartSprintInput);
			pEnhancedInput->BindAction(SprintActionAsset, ETriggerEvent::Completed, this, &ATPSPlayerController::StopSprintInput);
		}

		if (ensure(AimActionAsset))
		{
			pEnhancedInput->BindAction(AimActionAsset, ETriggerEvent::Started, this, &ATPSPlayerController::StartAimInput);
			pEnhancedInput->BindAction(AimActionAsset, ETriggerEvent::Completed, this, &ATPSPlayerController::StopAimInput);
		}
	}
}

void ATPSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ensure(InPawn))
	{
		MoveableInterface = InPawn;
		SprintableInterface = InPawn;
		AimableInterface = InPawn;
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

void ATPSPlayerController::StartSprintInput(const FInputActionValue& InputValue)
{
	if (ensure(SprintableInterface))
	{
		SprintableInterface->StartSprint();
	}
}

void ATPSPlayerController::StopSprintInput(const FInputActionValue& InputValue)
{
	if (ensure(SprintableInterface))
	{
		SprintableInterface->StopSprint();
	}
}

void ATPSPlayerController::StartAimInput(const FInputActionValue& InputValue)
{
	if (ensure(AimableInterface))
	{
		AimableInterface->StartAim();
	}
}

void ATPSPlayerController::StopAimInput(const FInputActionValue& InputValue)
{
	if (ensure(AimableInterface))
	{
		AimableInterface->StopAim();
	}
}