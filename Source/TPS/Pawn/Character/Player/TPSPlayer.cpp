#include "Pawn/Character/Player/TPSPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Component/Action/TPSCMC.h"
#include "Component/Action/TPSCameraControlComponent.h"
#include "Component/Action/TPSEquipComponent.h"
#include "Component/Data/TPSAnimLayerComponent.h"
#include "Component/Data/TPSPlayerStateComponent.h"
#include "Component/Data/TPSPlayerStatusComponent.h"
#include "Component/Action/TPSPlayerInteractionComponent.h"
#include "Component/Action/TPSFireComponent.h"
#include "Core/Controller/TPSPlayerController.h"

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

	if (ensure(AnimLayerComponentInst))
	{
		AnimLayerComponentInst->LinkAnimLayer(AnimLayerComponentInst->GetUnArmedLayerClass());
	}
}

void ATPSPlayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// ① CMC 초기화 (회전 모드 + 이동 속도)
	if (!CMCInst)
	{
		CMCInst = Cast<UTPSCMC>(GetCharacterMovement());
		if (ensure(CMCInst))
		{
			CMCInst->SetOrientRotationToMovement(true);
			CMCInst->SetRotationRate(FRotator(0.f, 700.f, 0.f));

			if (ensure(StatusComponentInst))
			{
				CMCInst->SetMaxWalkSpeed(StatusComponentInst->GetDefaultWalkSpeed());
			}
		}
	}

	// ② 카메라 컨트롤 초기화 (SpringArm + Camera 참조 전달)
	if (ensure(CameraControlComponentInst))
	{
		CameraControlComponentInst->Initialize(SpringArmComponentInst, CameraComponentInst);
	}

	// ③ 델리게이트 바인딩 (Equip, Fire 상태 변경 수신)
	BindDelegate();
}

void ATPSPlayer::CreateDefaultComponents()
{
	// ① 카메라 계열 (SpringArm → Camera → CameraControl)
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


	// ② 데이터 컴포넌트 (State, Status)
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

	// ③ 액션 컴포넌트 (Equip, AnimLayer, Interaction, Fire)
	if (!EquipComponentInst)
	{
		EquipComponentInst = CreateDefaultSubobject<UTPSEquipComponent>(TEXT("EquipComponent"));
		ensure(EquipComponentInst);
	}

	if (!AnimLayerComponentInst)
	{
		AnimLayerComponentInst = CreateDefaultSubobject<UTPSAnimLayerComponent>(TEXT("AnimLayerComponent"));
		ensure(AnimLayerComponentInst);
	}

	if (!InteractionComponentInst)
	{
		InteractionComponentInst = CreateDefaultSubobject<UTPSPlayerInteractionComponent>(TEXT("InteractionComponent"));
		ensure(InteractionComponentInst);
	}

	if (!FireComponentInst)
	{
		FireComponentInst = CreateDefaultSubobject<UTPSFireComponent>(TEXT("FireComponent"));
		ensure(FireComponentInst);
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

	if (!ensure(StateComponentInst) || !ensure(CMCInst)) return;

	const EMovementMode CurrentMode = CMCInst->MovementMode;

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

void ATPSPlayer::BindDelegate()
{
	if (ensure(EquipComponentInst))
	{
		if (!EquipComponentInst->OnEquipStateChangedDelegate.IsBoundToObject(this))
		{
			EquipComponentInst->OnEquipStateChangedDelegate.AddUObject(this, &ATPSPlayer::OnEquipStateChanged);
		}
	}

	if (ensure(FireComponentInst))
	{
		if (!FireComponentInst->OnFireStateChangedDelegate.IsBoundToObject(this))
		{
			FireComponentInst->OnFireStateChangedDelegate.AddUObject(this, &ATPSPlayer::OnFireStateChanged);
		}
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

	// ① 컨트롤러 Yaw 기준 전방/우측 방향 벡터 계산
	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	FRotationMatrix ControllerYawMatrix(YawRotation);
	const FVector ForwardDir = ControllerYawMatrix.GetUnitAxis(EAxis::X);
	const FVector RightDir = ControllerYawMatrix.GetUnitAxis(EAxis::Y);

	// ② 입력값 기반 이동 적용
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

	if (ensure(CMCInst) && ensure(StatusComponentInst))
	{
		CMCInst->UpdateSprintSpeed(StatusComponentInst->GetDefaultSprintSpeed());
		StateComponentInst->AddState(EActionState::Sprinting);
	}
}

void ATPSPlayer::StopSprint()
{
	if (!ensure(StateComponentInst)) return;

	if (!StateComponentInst->HasState(EActionState::Sprinting)) return;

	if (ensure(CMCInst) && ensure(StatusComponentInst))
	{
		CMCInst->UpdateSprintSpeed(StatusComponentInst->GetDefaultWalkSpeed());
	}

	StateComponentInst->RemoveState(EActionState::Sprinting);
}

void ATPSPlayer::StartAim()
{
	if (!ensure(StateComponentInst)) return;

	if (!StateComponentInst->HasState(EActionState::Equipping)) return;

	if (ensure(CameraControlComponentInst))
	{
		// ① ADS 카메라 전환 + 상태 추가
		CameraControlComponentInst->StartADS();
		StateComponentInst->AddState(EActionState::Aiming);

		// ② 스프린트 중이면 중단
		if (StateComponentInst->HasState(EActionState::Sprinting))
		{
			StopSprint();
		}

		// ③ 이동 방향 회전 해제 → 카메라 방향 고정
		if (ensure(CMCInst))
		{
			CMCInst->SetOrientRotationToMovement(false);
		}

		// ④ 보간 Tick 활성화 + ADS AnimLayer 전환
		SetInterpolateTickEnabled(true);
		if (ensure(AnimLayerComponentInst))
		{
			AnimLayerComponentInst->LinkAnimLayer(AnimLayerComponentInst->GetRifleADSLayerClass());
		}
	}
}

void ATPSPlayer::StopAim()
{
	if (!ensure(StateComponentInst)) return;

	if (!StateComponentInst->HasState(EActionState::Equipping)) return;

	if (ensure(CameraControlComponentInst))
	{
		// ① ADS 카메라 복원 + 상태 제거
		CameraControlComponentInst->StopADS();
		StateComponentInst->RemoveState(EActionState::Aiming);

		// ② 보간 Tick 비활성화 + HipFire AnimLayer 전환
		SetInterpolateTickEnabled(false);

		if (ensure(AnimLayerComponentInst))
		{
			AnimLayerComponentInst->LinkAnimLayer(AnimLayerComponentInst->GetRifleHipFireLayerClass());
		}
	}
}

void ATPSPlayer::Interpolate_Tick(float DeltaTime)
{
	// ① 목표 회전 계산 (컨트롤러 Yaw 방향)
	const FRotator CurrentRotation = GetActorRotation();
	const FRotator ControlRot = GetControlRotation();
	const FRotator TargetRotation(CurrentRotation.Pitch, ControlRot.Yaw, CurrentRotation.Roll);

	// ② 부드럽게 보간 적용
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, AimRotationInterpSpeed);
	SetActorRotation(NewRotation);

	// ③ 보간 완료 시 → 컨트롤러 직접 제어로 전환 + Tick 비활성화
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
	if (!ensure(StateComponentInst) || !ensure(EquipComponentInst)) return;

	// ① 사격 중이면 먼저 중단
	if (StateComponentInst->HasState(EActionState::Firing))
	{
		StopFire();
	}

	// ② 현재 장착 여부 확인 → 토글 요청
	const bool bIsCurrentlyEquipped = StateComponentInst->HasState(EActionState::Equipping);
	EquipComponentInst->RequestToggle(bIsCurrentlyEquipped);
}

void ATPSPlayer::Unequip()
{
	if (!ensure(StateComponentInst) || !ensure(EquipComponentInst)) return;

	if (StateComponentInst->HasState(EActionState::Equipping))
	{
		EquipComponentInst->RequestToggle(true);
	}
}

void ATPSPlayer::Interact()
{
	if (ensure(InteractionComponentInst))
	{
		InteractionComponentInst->HandleInteraction();
	}
}

void ATPSPlayer::StartFire()
{
	if (!ensure(StateComponentInst) || !ensure(EquipComponentInst) || !ensure(FireComponentInst)) return;

	// ① 장착 상태 확인
	if (!StateComponentInst->HasState(EActionState::Equipping)) return;

	// ② 무기 + 탄약 확인
	ATPSWeaponBase* pWeapon = EquipComponentInst->GetWeaponActor();
	if (!ensure(pWeapon)) return;

	if (!pWeapon->HasAmmo()) return;

	// ③ 뷰포인트 콜백 설정 → FireComponent에 사격 요청
	ATPSPlayerController* pController = Cast<ATPSPlayerController>(GetController());
	if (ensure(pController))
	{
		FireComponentInst->StartFire(pWeapon, [pController](FVector& OutLocation, FRotator& OutRotation)
		{
			pController->GetPlayerViewPoint(OutLocation, OutRotation);
		});
	}
}

void ATPSPlayer::StopFire()
{
	if (ensure(FireComponentInst))
	{
		FireComponentInst->StopFire();
	}
}

void ATPSPlayer::OnFireStateChanged(bool bIsFiring)
{
	if (!ensure(StateComponentInst)) return;

	if (bIsFiring)
	{
		StateComponentInst->AddState(EActionState::Firing);
	}
	else
	{
		StateComponentInst->RemoveState(EActionState::Firing);
	}
}

void ATPSPlayer::OnEquipStateChanged(bool bIsEquipped)
{
	if (!ensure(StateComponentInst) || !ensure(CMCInst)) return;

	if (bIsEquipped)
	{
		// ① 장착 — 상태 추가 + 카메라 방향 고정 + 보간 시작
		StateComponentInst->AddState(EActionState::Equipping);
		CMCInst->SetOrientRotationToMovement(false);
		SetInterpolateTickEnabled(true);
	}
	else
	{
		// ② 해제 — 사격/조준 진행 중이면 먼저 중단
		if (ensure(FireComponentInst) && FireComponentInst->GetIsFiring())
		{
			FireComponentInst->StopFire();
		}

		if (StateComponentInst->HasState(EActionState::Aiming))
		{
			StopAim();
		}

		// ③ 해제 — 상태 제거 + 이동 방향 회전 복원
		StateComponentInst->RemoveState(EActionState::Equipping);
		CMCInst->SetOrientRotationToMovement(true);
		bUseControllerRotationYaw = false;
	}
}