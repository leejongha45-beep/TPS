#include "Pawn/Character/Player/TPSPlayer.h"
#include "Animation/Player/TPSLinkedAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Component/Action/TPSCameraControlComponent.h"
#include "Component/Action/TPSCMC.h"
#include "Component/Data/TPSPlayerStateComponent.h"
#include "Component/Data/TPSPlayerStatusComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

DECLARE_LOG_CATEGORY_EXTERN(AimTickLog, Warning, All);

DEFINE_LOG_CATEGORY(AimTickLog);

ATPSPlayer::ATPSPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UTPSCMC>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	InterpolateTickFunction.bCanEverTick = true;
	InterpolateTickFunction.bStartWithTickEnabled = false;
	InterpolateTickFunction.TickGroup = TG_PrePhysics;

	CreateDefaultComponents();
}

void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (UnArmedAnimLayerClass)
	{
		LinkAnimLayer(UnArmedAnimLayerClass);
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
			CachedCMC->SetOrientRotationToMovement(true);

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

void ATPSPlayer::RegisterActorTickFunctions(bool bRegister)
{
	Super::RegisterActorTickFunctions(bRegister);

	if (bRegister)
	{
		InterpolateTickFunction.Target = this;
		InterpolateTickFunction.RegisterTickFunction(GetLevel());
		InterpolateTickFunction.SetTickFunctionEnable(InterpolateTickFunction.bStartWithTickEnabled);
	}
	else
	{
		if (InterpolateTickFunction.IsTickFunctionRegistered())
		{
			InterpolateTickFunction.UnRegisterTickFunction();
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
	if (!ensure(StateComponentInst)) return;

	if (!StateComponentInst->HasState(EActionState::Equipping)) return;

	if (ensure(CameraControlComponentInst))
	{
		CameraControlComponentInst->StartADS();
		StateComponentInst->AddState(EActionState::Aiming);

		if (StateComponentInst->HasState(EActionState::Sprinting))
		{
			StopSprint();
		}

		if (ensure(CachedCMC))
		{
			CachedCMC->SetOrientRotationToMovement(false);
		}

		SetInterpolateTickEnabled(true);
		LinkAnimLayer(RifleADSAnimLayerClass);
	}
}

void ATPSPlayer::StopAim()
{
	if (!ensure(StateComponentInst)) return;

	if (!StateComponentInst->HasState(EActionState::Equipping)) return;

	if (ensure(CameraControlComponentInst))
	{
		CameraControlComponentInst->StopADS();
		StateComponentInst->RemoveState(EActionState::Aiming);

		SetInterpolateTickEnabled(false);
		LinkAnimLayer(RifleHipFireAnimLayerClass);
	}
}

void ATPSPlayer::Interpolate_Tick(float DeltaTime)
{
	const FRotator CurrentRotation = GetActorRotation();
	const FRotator ControlRot = GetControlRotation();
	const FRotator TargetRotation(CurrentRotation.Pitch, ControlRot.Yaw, CurrentRotation.Roll);

	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, AimRotationInterpSpeed);
	SetActorRotation(NewRotation);

	if (FMath::IsNearlyEqual(FRotator::NormalizeAxis(NewRotation.Yaw), FRotator::NormalizeAxis(TargetRotation.Yaw), 1.f))
	{
		bUseControllerRotationYaw = true;
		SetInterpolateTickEnabled(false);
	}
}

void ATPSPlayer::SetInterpolateTickEnabled(bool bEnabled)
{
	if (InterpolateTickFunction.IsTickFunctionEnabled() == bEnabled) return;

	InterpolateTickFunction.SetTickFunctionEnable(bEnabled);
	UE_LOG(AimTickLog, Warning, TEXT("[SetInterpolateTickEnabled] %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void ATPSPlayer::StartJump()
{
	Jump();
}

void ATPSPlayer::StopJump()
{
	StopJumping();
}

void ATPSPlayer::Equip()
{
	if (!ensure(StateComponentInst)) return;

	if (StateComponentInst->HasState(EActionState::Equipping))
	{
		Unequip();
		return;
	}

	StateComponentInst->AddState(EActionState::Equipping);

	if (ensure(CachedCMC))
	{
		CachedCMC->SetOrientRotationToMovement(false);
	}

	SetInterpolateTickEnabled(true);
	LinkAnimLayer(RifleHipFireAnimLayerClass);
}

void ATPSPlayer::Unequip()
{
	if (!ensure(StateComponentInst)) return;

	if (StateComponentInst->HasState(EActionState::Aiming))
	{
		StopAim();
	}

	StateComponentInst->RemoveState(EActionState::Equipping);
	
	if (ensure(CachedCMC))
	{
		CachedCMC->SetOrientRotationToMovement(true);
	}
	
	bUseControllerRotationYaw = false;
	LinkAnimLayer(UnArmedAnimLayerClass);
}

void ATPSPlayer::LinkAnimLayer(TSubclassOf<UTPSLinkedAnimInstance> InClass)
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