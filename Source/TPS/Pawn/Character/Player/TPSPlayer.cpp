#include "Pawn/Character/Player/TPSPlayer.h"
#include "Component/Action/TPSCMC.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ATPSPlayer::ATPSPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UTPSCMC>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

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
			CachedCMC->SetOrientRotationToMovement(true);
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