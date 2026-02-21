#include "Character/Player/TPSPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Character/Component/Action/TPSCameraControlComponent.h"
#include "Character/Component/Action/TPSEquipComponent.h"
#include "Character/Component/Action/TPSFireComponent.h"
#include "Character/Component/Data/TPSAnimLayerComponent.h"
#include "Character/Component/Data/TPSPlayerStateComponent.h"
#include "Character/Component/Action/TPSPlayerInteractionComponent.h"
#include "Core/Controller/TPSPlayerController.h"
#include "Weapon/TPSWeaponBase.h"

DECLARE_LOG_CATEGORY_EXTERN(AimTickLog, Warning, All);

DEFINE_LOG_CATEGORY(AimTickLog);

ATPSPlayer::ATPSPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InterpolateTickFunction.bCanEverTick = true;
	InterpolateTickFunction.bStartWithTickEnabled = false;
	InterpolateTickFunction.TickGroup = TG_PrePhysics;

	ATPSPlayer::CreateDefaultComponents();
}

void ATPSPlayer::CreateDefaultComponents()
{
	Super::CreateDefaultComponents();

	// ③ Player 전용 컴포넌트 (카메라 계열 + Interaction)
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

	if (!InteractionComponentInst)
	{
		InteractionComponentInst = CreateDefaultSubobject<UTPSPlayerInteractionComponent>(TEXT("InteractionComponent"));
		ensure(InteractionComponentInst);
	}
}

void ATPSPlayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Player 전용: 카메라 컨트롤 초기화
	if (ensure(CameraControlComponentInst))
	{
		CameraControlComponentInst->Initialize(SpringArmComponentInst, CameraComponentInst);
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

void ATPSPlayer::Interact()
{
	if (ensure(InteractionComponentInst))
	{
		InteractionComponentInst->HandleInteraction();
	}
}

void ATPSPlayer::StartAim()
{
	if (!ensure(StateComponentInst)) return;

	if (!StateComponentInst->HasState(EActionState::Equipping)) return;

	if (ensure(CameraControlComponentInst))
	{
		// ① 공통 조준 처리 (State + Sprint중단 + CMC)
		Super::StartAim();

		// ② Player 전용: ADS 카메라 전환
		CameraControlComponentInst->StartADS();

		// ③ 보간 Tick 활성화 + ADS AnimLayer 전환
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
		// ① 공통 조준 해제 (State 제거)
		Super::StopAim();

		// ② Player 전용: ADS 카메라 복원
		CameraControlComponentInst->StopADS();

		// ③ 보간 Tick 비활성화 + HipFire AnimLayer 전환
		SetInterpolateTickEnabled(false);

		if (ensure(AnimLayerComponentInst))
		{
			AnimLayerComponentInst->LinkAnimLayer(AnimLayerComponentInst->GetRifleHipFireLayerClass());
		}
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

void ATPSPlayer::OnEquipStateChanged(bool bIsEquipped)
{
	if (bIsEquipped)
	{
		// ① 공통 처리 (State + CMC)
		Super::OnEquipStateChanged(bIsEquipped);

		// ② Player 전용: 보간 활성화
		SetInterpolateTickEnabled(true);

		// ③ AmmoViewModel 브로드캐스트 → HUD에서 수신
		if (ensure(EquipComponentInst))
		{
			ATPSWeaponBase* pWeapon = EquipComponentInst->GetWeaponActor();
			if (ensure(pWeapon))
			{
				OnAmmoViewModelChangedDelegate.Broadcast(pWeapon->GetAmmoViewModel());
			}
		}
	}
	else
	{
		// ① AmmoViewModel 해제 브로드캐스트 (Super 전에 — Super가 사격/조준 중단 처리)
		OnAmmoViewModelChangedDelegate.Broadcast(nullptr);

		// ② 공통 처리 (State + CMC + 사격/조준 중단)
		Super::OnEquipStateChanged(bIsEquipped);

		// ③ Player 전용: 컨트롤러 Yaw 해제
		bUseControllerRotationYaw = false;
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