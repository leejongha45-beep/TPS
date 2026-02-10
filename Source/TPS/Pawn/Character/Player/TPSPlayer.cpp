#include "Pawn/Character/Player/TPSPlayer.h"
#include "Component/Action/TPSCMC.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ATPSPlayer::ATPSPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UTPSCMC>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	CreateDefaultComponents();
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
			CachedCMC->SetMaxWalkSpeed(500.f);
		}
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
			SpringArmComponentInst->TargetArmLength = 300.0f;
			SpringArmComponentInst->bUsePawnControlRotation = true;

			if (!CameraComponentInst)
			{
				CameraComponentInst = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
				if (ensure(CameraComponentInst))
				{
					CameraComponentInst->SetupAttachment(SpringArmComponentInst);
					CameraComponentInst->bUsePawnControlRotation = false;
				}
			}
		}
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