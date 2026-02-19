#include "Core/Controller/TPSPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Utils/Interface/Action/Aimable.h"
#include "Utils/Interface/Action/Moveable.h"
#include "Utils/Interface/Action/Jumpable.h"
#include "Utils/Interface/Action/Sprintable.h"
#include "Utils/Interface/Action/Equippable.h"
#include "Utils/Interface/Action/Interactable.h"
#include "Utils/Interface/Action/Fireable.h"

ATPSPlayerController::ATPSPlayerController()
{
	SetShowMouseCursor(false);
}

void ATPSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// ① IMC (Input Mapping Context) 등록
	UEnhancedInputLocalPlayerSubsystem* pSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (ensure(pSubsystem))
	{
		if (ensure(DefaultMappingContextAsset))
		{
			pSubsystem->AddMappingContext(DefaultMappingContextAsset, 0);
		}
	}

	// ② 입력 액션 바인딩 (Look, Move, Sprint, Aim, Jump, Equip, Interact, Fire)
	UEnhancedInputComponent* pEnhancedInput = CastChecked<UEnhancedInputComponent>(InputComponent);
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

	if (ensure(JumpActionAsset))
	{
		pEnhancedInput->BindAction(JumpActionAsset, ETriggerEvent::Started, this, &ATPSPlayerController::StartJumpInput);
		pEnhancedInput->BindAction(JumpActionAsset, ETriggerEvent::Completed, this, &ATPSPlayerController::StopJumpInput);
	}

	if (ensure(EquipActionAsset))
	{
		pEnhancedInput->BindAction(EquipActionAsset, ETriggerEvent::Started, this, &ATPSPlayerController::EquipInput);
	}

	if (ensure(InteractActionAsset))
	{
		pEnhancedInput->BindAction(InteractActionAsset, ETriggerEvent::Started, this, &ATPSPlayerController::InteractInput);
	}

	if (ensure(FireActionAsset))
	{
		pEnhancedInput->BindAction(FireActionAsset, ETriggerEvent::Started, this, &ATPSPlayerController::StartFireInput);
		pEnhancedInput->BindAction(FireActionAsset, ETriggerEvent::Completed, this, &ATPSPlayerController::StopFireInput);
	}

	if (ensure(ReloadActionAsset))
	{
		pEnhancedInput->BindAction(ReloadActionAsset, ETriggerEvent::Started, this, &ATPSPlayerController::ReloadInput);
	}
}

void ATPSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// TScriptInterface 캐싱 — 모든 액션 인터페이스를 Pawn에서 획득
	if (ensure(InPawn))
	{
		if (!MoveableInterface) MoveableInterface = InPawn;
		ensure(MoveableInterface);

		if (!SprintableInterface) SprintableInterface = InPawn;
		ensure(SprintableInterface);

		if (!AimableInterface) AimableInterface = InPawn;
		ensure(AimableInterface);

		if (!JumpableInterface) JumpableInterface = InPawn;
		ensure(JumpableInterface);

		if (!EquippableInterface) EquippableInterface = InPawn;
		ensure(EquippableInterface);

		if (!InteractableInterface) InteractableInterface = InPawn;
		ensure(InteractableInterface);

		if (!FireableInterface) FireableInterface = InPawn;
		ensure(FireableInterface);
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

void ATPSPlayerController::StartJumpInput(const FInputActionValue& InputValue)
{
	if (ensure(JumpableInterface))
	{
		JumpableInterface->StartJump();
	}
}

void ATPSPlayerController::StopJumpInput(const FInputActionValue& InputValue)
{
	if (ensure(JumpableInterface))
	{
		JumpableInterface->StopJump();
	}
}

void ATPSPlayerController::EquipInput(const FInputActionValue& InputValue)
{
	if (ensure(EquippableInterface))
	{
		EquippableInterface->Equip();
	}
}

void ATPSPlayerController::InteractInput(const FInputActionValue& InputValue)
{
	if (ensure(InteractableInterface))
	{
		InteractableInterface->Interact();
	}
}

void ATPSPlayerController::StartFireInput(const FInputActionValue& InputValue)
{
	if (ensure(FireableInterface))
	{
		FireableInterface->StartFire();
	}
}

void ATPSPlayerController::StopFireInput(const FInputActionValue& InputValue)
{
	if (ensure(FireableInterface))
	{
		FireableInterface->StopFire();
	}
}

void ATPSPlayerController::ReloadInput(const FInputActionValue& InputValue)
{
	if (ensure(FireableInterface))
	{
		FireableInterface->Reload();
	}
}