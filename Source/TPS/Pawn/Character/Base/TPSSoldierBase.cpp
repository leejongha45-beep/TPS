#include "Pawn/Character/Base/TPSSoldierBase.h"
#include "Component/Action/TPSEquipComponent.h"
#include "Component/Action/TPSFireComponent.h"
#include "Component/Data/TPSAnimLayerComponent.h"
#include "Component/Data/TPSPlayerStateComponent.h"
#include "Component/Action/TPSCMC.h"

ATPSSoldierBase::ATPSSoldierBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ATPSSoldierBase::CreateDefaultComponents();
}

void ATPSSoldierBase::CreateDefaultComponents()
{
	Super::CreateDefaultComponents();

	// ② 전투 컴포넌트 (Equip, AnimLayer, Fire)
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

	if (!FireComponentInst)
	{
		FireComponentInst = CreateDefaultSubobject<UTPSFireComponent>(TEXT("FireComponent"));
		ensure(FireComponentInst);
	}
}

void ATPSSoldierBase::BeginPlay()
{
	Super::BeginPlay();

	if (ensure(AnimLayerComponentInst))
	{
		AnimLayerComponentInst->LinkAnimLayer(AnimLayerComponentInst->GetUnArmedLayerClass());
	}
}

void ATPSSoldierBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	BindDelegate();
}

void ATPSSoldierBase::BindDelegate()
{
	if (ensure(EquipComponentInst))
	{
		if (!EquipComponentInst->OnEquipStateChangedDelegate.IsBoundToObject(this))
		{
			EquipComponentInst->OnEquipStateChangedDelegate.AddUObject(this, &ATPSSoldierBase::OnEquipStateChanged);
		}
	}

	if (ensure(FireComponentInst))
	{
		if (!FireComponentInst->OnFireStateChangedDelegate.IsBoundToObject(this))
		{
			FireComponentInst->OnFireStateChangedDelegate.AddUObject(this, &ATPSSoldierBase::OnFireStateChanged);
		}
	}
}

void ATPSSoldierBase::StartAim()
{
	if (!ensure(StateComponentInst)) return;

	if (!StateComponentInst->HasState(EActionState::Equipping)) return;

	// 공통: 상태 추가 + 스프린트 중단 + CMC 회전모드
	StateComponentInst->AddState(EActionState::Aiming);

	if (StateComponentInst->HasState(EActionState::Sprinting))
	{
		StopSprint();
	}

	if (ensure(CMCInst))
	{
		CMCInst->SetOrientRotationToMovement(false);
	}
}

void ATPSSoldierBase::StopAim()
{
	if (!ensure(StateComponentInst)) return;

	if (!StateComponentInst->HasState(EActionState::Equipping)) return;

	// 공통: 상태 제거
	StateComponentInst->RemoveState(EActionState::Aiming);
}

void ATPSSoldierBase::Equip()
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

void ATPSSoldierBase::Unequip()
{
	if (!ensure(StateComponentInst) || !ensure(EquipComponentInst)) return;

	if (StateComponentInst->HasState(EActionState::Equipping))
	{
		EquipComponentInst->RequestToggle(true);
	}
}

void ATPSSoldierBase::StartFire()
{
	// 기본 구현: 비어있음 — Player/NPC가 각각 override
	// Player: ViewPoint 람다 전달
	// NPC: AI 기반 방향 계산
}

void ATPSSoldierBase::StopFire()
{
	if (ensure(FireComponentInst))
	{
		FireComponentInst->StopFire();
	}
}

void ATPSSoldierBase::OnEquipStateChanged(bool bIsEquipped)
{
	if (!ensure(StateComponentInst) || !ensure(CMCInst)) return;

	if (bIsEquipped)
	{
		// 공통: 상태 추가 + CMC 회전모드 변경
		StateComponentInst->AddState(EActionState::Equipping);
		CMCInst->SetOrientRotationToMovement(false);
	}
	else
	{
		// 공통: 사격/조준 중단 + 상태 제거 + CMC 복원
		if (ensure(FireComponentInst) && FireComponentInst->GetIsFiring())
		{
			FireComponentInst->StopFire();
		}

		if (StateComponentInst->HasState(EActionState::Aiming))
		{
			StopAim();
		}

		StateComponentInst->RemoveState(EActionState::Equipping);
		CMCInst->SetOrientRotationToMovement(true);
	}
}

void ATPSSoldierBase::OnFireStateChanged(bool bIsFiring)
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