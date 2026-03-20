#include "Core/Controller/TPSPlayerController.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
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
#include "UI/HUD/TPSHUD.h"

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
		if (ensure(DefaultMappingContextAsset.Get()))
		{
			pSubsystem->AddMappingContext(DefaultMappingContextAsset.Get(), 0);
		}
	}

	// ② 입력 액션 바인딩 (Look, Move, Sprint, Aim, Jump, Equip, Interact, Fire)
	UEnhancedInputComponent* pEnhancedInput = CastChecked<UEnhancedInputComponent>(InputComponent);
	if (ensure(LookActionAsset.Get()))
	{
		pEnhancedInput->BindAction(LookActionAsset.Get(), ETriggerEvent::Triggered, this, &ATPSPlayerController::Look);
	}

	if (ensure(MoveActionAsset.Get()))
	{
		pEnhancedInput->BindAction(MoveActionAsset.Get(), ETriggerEvent::Started, this, &ATPSPlayerController::StartMoveInput);
		pEnhancedInput->BindAction(MoveActionAsset.Get(), ETriggerEvent::Triggered, this, &ATPSPlayerController::MoveInput);
		pEnhancedInput->BindAction(MoveActionAsset.Get(), ETriggerEvent::Completed, this, &ATPSPlayerController::StopMoveInput);
	}

	if (ensure(SprintActionAsset.Get()))
	{
		pEnhancedInput->BindAction(SprintActionAsset.Get(), ETriggerEvent::Started, this, &ATPSPlayerController::StartSprintInput);
		pEnhancedInput->BindAction(SprintActionAsset.Get(), ETriggerEvent::Completed, this, &ATPSPlayerController::StopSprintInput);
	}

	if (ensure(AimActionAsset.Get()))
	{
		pEnhancedInput->BindAction(AimActionAsset.Get(), ETriggerEvent::Started, this, &ATPSPlayerController::StartAimInput);
		pEnhancedInput->BindAction(AimActionAsset.Get(), ETriggerEvent::Completed, this, &ATPSPlayerController::StopAimInput);
	}

	if (ensure(JumpActionAsset.Get()))
	{
		pEnhancedInput->BindAction(JumpActionAsset.Get(), ETriggerEvent::Started, this, &ATPSPlayerController::StartJumpInput);
		pEnhancedInput->BindAction(JumpActionAsset.Get(), ETriggerEvent::Completed, this, &ATPSPlayerController::StopJumpInput);
	}

	if (ensure(EquipActionAsset.Get()))
	{
		pEnhancedInput->BindAction(EquipActionAsset.Get(), ETriggerEvent::Started, this, &ATPSPlayerController::EquipInput);
	}

	if (ensure(InteractActionAsset.Get()))
	{
		pEnhancedInput->BindAction(InteractActionAsset.Get(), ETriggerEvent::Started, this, &ATPSPlayerController::InteractInput);
	}

	if (ensure(FireActionAsset.Get()))
	{
		pEnhancedInput->BindAction(FireActionAsset.Get(), ETriggerEvent::Started, this, &ATPSPlayerController::StartFireInput);
		pEnhancedInput->BindAction(FireActionAsset.Get(), ETriggerEvent::Completed, this, &ATPSPlayerController::StopFireInput);
	}

	if (ensure(ReloadActionAsset.Get()))
	{
		pEnhancedInput->BindAction(ReloadActionAsset.Get(), ETriggerEvent::Started, this, &ATPSPlayerController::ReloadInput);
	}

	if (MinimapActionAsset.Get())
	{
		pEnhancedInput->BindAction(MinimapActionAsset.Get(), ETriggerEvent::Started, this, &ATPSPlayerController::MinimapInput);
	}
}

void ATPSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// TScriptInterface 캐싱 — 모든 액션 인터페이스를 Pawn에서 획득
	if (ensure(InPawn))
	{
		if (!MoveableInterface)
		{
			MoveableInterface.SetObject(InPawn);
			MoveableInterface.SetInterface(Cast<IMoveable>(InPawn));
		}
		ensure(MoveableInterface);

		if (!SprintableInterface)
		{
			SprintableInterface.SetObject(InPawn);
			SprintableInterface.SetInterface(Cast<ISprintable>(InPawn));
		}
		ensure(SprintableInterface);

		if (!AimableInterface)
		{
			AimableInterface.SetObject(InPawn);
			AimableInterface.SetInterface(Cast<IAimable>(InPawn));
		}
		ensure(AimableInterface);

		if (!JumpableInterface)
		{
			JumpableInterface.SetObject(InPawn);
			JumpableInterface.SetInterface(Cast<IJumpable>(InPawn));
		}
		ensure(JumpableInterface);

		if (!EquippableInterface)
		{
			EquippableInterface.SetObject(InPawn);
			EquippableInterface.SetInterface(Cast<IEquippable>(InPawn));
		}
		ensure(EquippableInterface);

		if (!InteractableInterface)
		{
			InteractableInterface.SetObject(InPawn);
			InteractableInterface.SetInterface(Cast<IInteractable>(InPawn));
		}
		ensure(InteractableInterface);

		if (!FireableInterface)
		{
			FireableInterface.SetObject(InPawn);
			FireableInterface.SetInterface(Cast<IFireable>(InPawn));
		}
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

void ATPSPlayerController::MinimapInput(const FInputActionValue& InputValue)
{
	ATPSHUD* pHUD = Cast<ATPSHUD>(GetHUD());
	if (pHUD)
	{
		pHUD->ToggleMinimap();
	}
}