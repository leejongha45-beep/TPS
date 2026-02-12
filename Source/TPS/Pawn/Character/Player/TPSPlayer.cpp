#include "Pawn/Character/Player/TPSPlayer.h"
#include "Camera/CameraComponent.h"
#include "Component/Action/TPSCMC.h"
#include "Component/Action/TPSCameraControlComponent.h"
#include "Component/Data/TPSPlayerStateComponent.h"
#include "Component/Data/TPSPlayerStatusComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

ATPSPlayer::ATPSPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UTPSCMC>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	CreateDefaultComponents();
}

void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (DefaultAnimLayerClass)
	{
		LinkAnimLayer(DefaultAnimLayerClass);
	}
}

void ATPSPlayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!CachedCMC)
	{
		CachedCMC = Cast<UTPSCMC>(GetCharacterMovement());
		if (ensure(CachedCMC))
		{
			CachedCMC->SetOrientRotationToMovement(false);

			if (ensure(StatusComponentInst))
			{
				CachedCMC->SetMaxWalkSpeed(StatusComponentInst->GetDefaultWalkSpeed());
			}
		}
	}

	if (ensure(CameraControlComponentInst))
	{
		CameraControlComponentInst->Initialize(SpringArmComponentInst, CameraComponentInst);
	}
}

void ATPSPlayer::CreateDefaultComponents()
{
	if (!SpringArmComponentInst)
	{
		SpringArmComponentInst = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
		if (ensure(SpringArmComponentInst))
		{
			SpringArmComponentInst->SetupAttachment(RootComponent);
			SpringArmComponentInst->SetRelativeLocation(FVector(0.f, 20.f, 60.f));
			SpringArmComponentInst->bUsePawnControlRotation = true;

			if (!CameraComponentInst)
			{
				CameraComponentInst = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
				if (ensure(CameraComponentInst))
				{
					CameraComponentInst->SetupAttachment(SpringArmComponentInst);
					CameraComponentInst->bUsePawnControlRotation = false;
					
					if (!CameraControlComponentInst)
					{
						CameraControlComponentInst = CreateDefaultSubobject<UTPSCameraControlComponent>(TEXT("CameraControlComponent"));
						ensure(CameraControlComponentInst);
					}
				}
			}
		}
	}

	
	if (!StateComponentInst)
	{
		StateComponentInst = CreateDefaultSubobject<UTPSPlayerStateComponent>(TEXT("StateComponent"));
		if (ensure(StateComponentInst))
		{
			StateComponentInst->ClearState();
			StateComponentInst->AddState(EActionState::Idle);
		}
	}

	if (!StatusComponentInst)
	{
		StatusComponentInst = CreateDefaultSubobject<UTPSPlayerStatusComponent>(TEXT("StatusComponent"));
		if (ensure(StatusComponentInst))
		{
			StatusComponentInst->SetDefaultSprintSpeed(700.f);
			StatusComponentInst->SetDefaultWalkSpeed(500.f);
		}
	}
}

void ATPSPlayer::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	if (ensure(StateComponentInst))
	{
		StateComponentInst->AddState(EActionState::Jumping);
	}
}

void ATPSPlayer::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (!ensure(StateComponentInst) || !ensure(CachedCMC)) return;

	const EMovementMode CurrentMode = CachedCMC->MovementMode;

	if (CurrentMode == MOVE_Falling)
	{
		StateComponentInst->AddState(EActionState::Falling);
	}
	else if (PrevMovementMode == MOVE_Falling && CurrentMode == MOVE_Walking)
	{
		StateComponentInst->RemoveState(EActionState::Falling);
		StateComponentInst->RemoveState(EActionState::Jumping);
	}
}

void ATPSPlayer::StartMove()
{
	if (ensure(StateComponentInst))
	{
		StateComponentInst->RemoveState(EActionState::Idle);
		StateComponentInst->AddState(EActionState::Moving);
	}
}

void ATPSPlayer::Move(const FVector2D& InputVector)
{
	if (InputVector.IsZero()) return;

	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	FRotationMatrix ControllerYawMatrix(YawRotation);
	const FVector ForwardDir = ControllerYawMatrix.GetUnitAxis(EAxis::X);
	const FVector RightDir = ControllerYawMatrix.GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, InputVector.X);
	AddMovementInput(RightDir, InputVector.Y);
}

void ATPSPlayer::StopMove()
{
	if (ensure(StateComponentInst))
	{
		StateComponentInst->RemoveState(EActionState::Moving);

		if (StateComponentInst->HasState(EActionState::Sprinting))
		{
			StopSprint();
		}

		StateComponentInst->AddState(EActionState::Idle);
	}
}

void ATPSPlayer::StartSprint()
{
	if (!ensure(StateComponentInst)) return;

	if (!StateComponentInst->HasState(EActionState::Moving) || StateComponentInst->HasState(EActionState::Aiming)) return;

	if (ensure(CachedCMC) && ensure(StatusComponentInst))
	{
		CachedCMC->UpdateSprintSpeed(StatusComponentInst->GetDefaultSprintSpeed());
		StateComponentInst->AddState(EActionState::Sprinting);
	}
}

void ATPSPlayer::StopSprint()
{
	if (!ensure(StateComponentInst)) return;

	if (!StateComponentInst->HasState(EActionState::Sprinting)) return;

	if (ensure(CachedCMC) && ensure(StatusComponentInst))
	{
		CachedCMC->UpdateSprintSpeed(StatusComponentInst->GetDefaultWalkSpeed());
	}

	StateComponentInst->RemoveState(EActionState::Sprinting);
}

void ATPSPlayer::StartAim()
{
	if (ensure(CameraControlComponentInst) && ensure(StateComponentInst))
	{
		CameraControlComponentInst->StartADS();
		StateComponentInst->AddState(EActionState::Aiming);

		if (StateComponentInst->HasState(EActionState::Sprinting))
		{
			StopSprint();
		}
	}
}

void ATPSPlayer::StopAim()
{
	if (ensure(CameraControlComponentInst) && ensure(StateComponentInst))
	{
		CameraControlComponentInst->StopADS();
		StateComponentInst->RemoveState(EActionState::Aiming);
	}
}

void ATPSPlayer::StartJump()
{
	Jump();
}

void ATPSPlayer::StopJump()
{
	StopJumping();
}

void ATPSPlayer::LinkAnimLayer(TSubclassOf<UAnimInstance> InClass)
{
	if (!InClass || InClass == CurrentAnimLayerClass) return;

	GetMesh()->LinkAnimClassLayers(InClass);
	CurrentAnimLayerClass = InClass;
}

void ATPSPlayer::UnlinkAnimLayer()
{
	if (!CurrentAnimLayerClass) return;

	GetMesh()->UnlinkAnimClassLayers(CurrentAnimLayerClass);
	CurrentAnimLayerClass = nullptr;
}