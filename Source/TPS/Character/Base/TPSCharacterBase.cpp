#include "Character/Base/TPSCharacterBase.h"
#include "Engine/World.h"
#include "Character/Component/Action/TPSCMC.h"
#include "Character/Component/Data/TPSPlayerStateComponent.h"
#include "Character/Component/Data/TPSPlayerStatusComponent.h"
#include "Character/Component/Data/TPSFootstepComponent.h"
#include "Core/Subsystem/TPSTargetSubsystem.h"
#include "Core/Subsystem/TPSDamageSubsystem.h"
#include "ECS/Scheduler/EnemyManagerSubsystem.h"
#include "GameFramework/Pawn.h"

ATPSCharacterBase::ATPSCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UTPSCMC>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	ATPSCharacterBase::CreateDefaultComponents();
}

void ATPSCharacterBase::CreateDefaultComponents()
{
	// ① 데이터 컴포넌트 (State, Status, Footstep)
	if (!StateComponentInst)
	{
		StateComponentInst = CreateDefaultSubobject<UTPSPlayerStateComponent>(TEXT("StateComponent"));
		if (ensure(StateComponentInst.Get()))
		{
			StateComponentInst->ClearState();
			StateComponentInst->AddState(EActionState::Idle);
		}
	}

	if (!StatusComponentInst)
	{
		StatusComponentInst = CreateDefaultSubobject<UTPSPlayerStatusComponent>(TEXT("StatusComponent"));
		if (ensure(StatusComponentInst.Get()))
		{
			StatusComponentInst->SetDefaultSprintSpeed(700.f);
			StatusComponentInst->SetDefaultWalkSpeed(500.f);
		}
	}

	if (!FootstepComponentInst)
	{
		FootstepComponentInst = CreateDefaultSubobject<UTPSFootstepComponent>(TEXT("FootstepComponent"));
		ensure(FootstepComponentInst.Get());
	}
}

void ATPSCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// ITargetable 등록 — Player/NPC 모든 자식 클래스 자동 등록
	if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
	{
		TargetSS->RegisterTargetableActor(TScriptInterface<ITargetable>(this));
	}

	// IDamageable 등록
	if (UTPSDamageSubsystem* DamageSS = GetWorld()->GetSubsystem<UTPSDamageSubsystem>())
	{
		DamageSS->RegisterDamageableActor(TScriptInterface<IDamageable>(this));
	}
}

void ATPSCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UTPSTargetSubsystem* TargetSS = GetWorld()->GetSubsystem<UTPSTargetSubsystem>())
	{
		TargetSS->UnregisterTargetableActor(TScriptInterface<ITargetable>(this));
	}

	if (UTPSDamageSubsystem* DamageSS = GetWorld()->GetSubsystem<UTPSDamageSubsystem>())
	{
		DamageSS->UnregisterDamageableActor(TScriptInterface<IDamageable>(this));
	}

	Super::EndPlay(EndPlayReason);
}

void ATPSCharacterBase::PostInitializeComponents()
{
	// ① CMC 초기화 — Super 전에 캐싱 (Possess → OnMovementModeChanged에서 필요)
	if (!CMCInst)
	{
		CMCInst = Cast<UTPSCMC>(GetCharacterMovement());
	}

	Super::PostInitializeComponents();

	if (ensure(CMCInst.Get()))
	{
		CMCInst->SetOrientRotationToMovement(true);
		CMCInst->SetRotationRate(FRotator(0.f, 700.f, 0.f));

		if (ensure(StatusComponentInst.Get()))
		{
			CMCInst->SetMaxWalkSpeed(StatusComponentInst->GetDefaultWalkSpeed());
		}
	}
}

void ATPSCharacterBase::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	if (ensure(StateComponentInst.Get()))
	{
		StateComponentInst->AddState(EActionState::Jumping);
	}
}

void ATPSCharacterBase::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (!ensure(StateComponentInst.Get()) || !ensure(CMCInst.Get())) return;

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

void ATPSCharacterBase::StartMove()
{
	if (ensure(StateComponentInst.Get()))
	{
		StateComponentInst->RemoveState(EActionState::Idle);
		StateComponentInst->AddState(EActionState::Moving);
	}
}

void ATPSCharacterBase::Move(const FVector2D& InputVector)
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

void ATPSCharacterBase::StopMove()
{
	if (ensure(StateComponentInst.Get()))
	{
		StateComponentInst->RemoveState(EActionState::Moving);

		if (StateComponentInst->HasState(EActionState::Sprinting))
		{
			StopSprint();
		}

		StateComponentInst->AddState(EActionState::Idle);
	}
}

void ATPSCharacterBase::StartSprint()
{
	if (!ensure(StateComponentInst.Get())) return;

	if (!StateComponentInst->HasState(EActionState::Moving) || StateComponentInst->HasState(EActionState::Aiming)) return;

	if (ensure(CMCInst.Get()) && ensure(StatusComponentInst.Get()))
	{
		CMCInst->UpdateSprintSpeed(StatusComponentInst->GetDefaultSprintSpeed());
		StateComponentInst->AddState(EActionState::Sprinting);
	}
}

void ATPSCharacterBase::StopSprint()
{
	if (!ensure(StateComponentInst.Get())) return;

	if (!StateComponentInst->HasState(EActionState::Sprinting)) return;

	if (ensure(CMCInst.Get()) && ensure(StatusComponentInst.Get()))
	{
		CMCInst->UpdateSprintSpeed(StatusComponentInst->GetDefaultWalkSpeed());
	}

	StateComponentInst->RemoveState(EActionState::Sprinting);
}

void ATPSCharacterBase::StartJump()
{
	Jump();
}

void ATPSCharacterBase::StopJump()
{
	StopJumping();
}

void ATPSCharacterBase::Interact()
{
	// 기본 구현: 비어있음 — 자식에서 override
}

float ATPSCharacterBase::ReceiveDamage(float Damage, AActor* DamageCauser)
{
	if (IsDead()) return 0.f;

	const float FinalDamage = ProcessDamage(Damage, DamageCauser);
	const bool bWasAlive = !IsDead();
	ApplyDamageToHP(FinalDamage);

	// NPC 사망 감지 → DamageCauser의 Instigator가 플레이어면 킬 이벤트 브로드캐스트
	if (bWasAlive && IsDead() && DamageCauser)
	{
		APawn* pInstigator = DamageCauser->GetInstigator();
		if (pInstigator && pInstigator->IsPlayerControlled())
		{
			if (UWorld* pWorld = GetWorld())
			{
				if (UEnemyManagerSubsystem* pEnemyMgr = pWorld->GetSubsystem<UEnemyManagerSubsystem>())
				{
					pEnemyMgr->OnPlayerKillECSDelegate.Broadcast(1);
					UE_LOG(LogTemp, Log, TEXT("[TPSCharacterBase] NPC killed by player — PsychoSync kill broadcast."));
				}
			}
		}
	}

	return FinalDamage;
}

// TODO: StatusComponent에 HP 추가 시 자식에서 override
float ATPSCharacterBase::ProcessDamage(float Damage, AActor* DamageCauser)
{
	return Damage;
}

void ATPSCharacterBase::ApplyDamageToHP(float FinalDamage)
{
	if (ensure(StatusComponentInst.Get()))
	{
		StatusComponentInst->SetCurrentHP(StatusComponentInst->GetCurrentHP() - FinalDamage);
	}
}

bool ATPSCharacterBase::IsDead() const
{
	if (ensure(StatusComponentInst.Get()))
	{
		return StatusComponentInst->GetCurrentHP() <= 0.f;
	}
	return false;
}