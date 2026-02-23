#include "Character/Base/TPSSoldierBase.h"
#include "Character/Component/Action/TPSEquipComponent.h"
#include "Character/Component/Action/TPSFireComponent.h"
#include "Character/Component/Data/TPSAnimLayerComponent.h"
#include "Character/Component/Data/TPSPlayerStateComponent.h"
#include "Character/Component/Action/TPSCMC.h"

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
		ensure(EquipComponentInst.Get());
	}

	if (!AnimLayerComponentInst)
	{
		AnimLayerComponentInst = CreateDefaultSubobject<UTPSAnimLayerComponent>(TEXT("AnimLayerComponent"));
		ensure(AnimLayerComponentInst.Get());
	}

	if (!FireComponentInst)
	{
		FireComponentInst = CreateDefaultSubobject<UTPSFireComponent>(TEXT("FireComponent"));
		ensure(FireComponentInst.Get());
	}
}

void ATPSSoldierBase::BeginPlay()
{
	Super::BeginPlay();

	if (ensure(AnimLayerComponentInst.Get()))
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
	if (ensure(EquipComponentInst.Get()))
	{
		if (!EquipComponentInst->OnEquipStateChangedDelegate.IsBoundToObject(this))
		{
			EquipComponentInst->OnEquipStateChangedDelegate.AddUObject(this, &ATPSSoldierBase::OnEquipStateChanged);
		}
	}

	if (ensure(FireComponentInst.Get()))
	{
		if (!FireComponentInst->OnFireStateChangedDelegate.IsBoundToObject(this))
		{
			FireComponentInst->OnFireStateChangedDelegate.AddUObject(this, &ATPSSoldierBase::OnFireStateChanged);
		}

		if (!FireComponentInst->OnReloadStateChangedDelegate.IsBoundToObject(this))
		{
			FireComponentInst->OnReloadStateChangedDelegate.AddUObject(this, &ATPSSoldierBase::OnReloadStateChanged);
		}
	}
}

void ATPSSoldierBase::StartAim()
{
	if (!ensure(StateComponentInst.Get())) return;

	if (!StateComponentInst->HasState(EActionState::Equipping)) return;

	// 공통: 상태 추가 + 스프린트 중단 + CMC 회전모드
	StateComponentInst->AddState(EActionState::Aiming);

	if (StateComponentInst->HasState(EActionState::Sprinting))
	{
		StopSprint();
	}

	if (ensure(CMCInst.Get()))
	{
		CMCInst->SetOrientRotationToMovement(false);
	}
}

void ATPSSoldierBase::StopAim()
{
	if (!ensure(StateComponentInst.Get())) return;

	if (!StateComponentInst->HasState(EActionState::Equipping)) return;

	// 공통: 상태 제거
	StateComponentInst->RemoveState(EActionState::Aiming);
}

void ATPSSoldierBase::Equip()
{
	if (!ensure(StateComponentInst.Get()) || !ensure(EquipComponentInst.Get())) return;

	// ① 사격 중이면 먼저 중단
	if (StateComponentInst->HasState(EActionState::Firing))
	{
		StopFire();
	}

	// ② 재장전 중이면 취소
	if (StateComponentInst->HasState(EActionState::Reloading))
	{
		if (ensure(FireComponentInst.Get()))
		{
			FireComponentInst->CancelReload();
		}
	}

	// ③ 현재 장착 여부 확인 → 토글 요청
	const bool bIsCurrentlyEquipped = StateComponentInst->HasState(EActionState::Equipping);
	EquipComponentInst->RequestToggle(bIsCurrentlyEquipped);
}

void ATPSSoldierBase::Unequip()
{
	if (!ensure(StateComponentInst.Get()) || !ensure(EquipComponentInst.Get())) return;

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
	if (ensure(FireComponentInst.Get()))
	{
		FireComponentInst->StopFire();
	}
}

void ATPSSoldierBase::OnEquipStateChanged(bool bIsEquipped)
{
	if (!ensure(StateComponentInst.Get()) || !ensure(CMCInst.Get())) return;

	if (bIsEquipped)
	{
		// 공통: 상태 추가 + CMC 회전모드 변경
		StateComponentInst->AddState(EActionState::Equipping);
		CMCInst->SetOrientRotationToMovement(false);
	}
	else
	{
		// 공통: 사격/재장전/조준 중단 + 상태 제거 + CMC 복원
		if (ensure(FireComponentInst.Get()))
		{
			if (FireComponentInst->GetIsFiring())
			{
				FireComponentInst->StopFire();
			}

			if (FireComponentInst->GetIsReloading())
			{
				FireComponentInst->CancelReload();
			}
		}

		if (StateComponentInst->HasState(EActionState::Aiming))
		{
			StopAim();
		}

		StateComponentInst->RemoveState(EActionState::Equipping);
		CMCInst->SetOrientRotationToMovement(true);
	}
}

void ATPSSoldierBase::Reload()
{
	if (!ensure(StateComponentInst.Get()) || !ensure(EquipComponentInst.Get()) || !ensure(FireComponentInst.Get())) return;

	// ① 장착 상태 확인
	if (!StateComponentInst->HasState(EActionState::Equipping)) return;

	// ② 이미 재장전 중이면 무시
	if (StateComponentInst->HasState(EActionState::Reloading)) return;

	// ③ 사격 중이면 먼저 중단
	if (StateComponentInst->HasState(EActionState::Firing))
	{
		StopFire();
	}

	// ④ 무기 확인 → FireComponent에 재장전 요청
	ATPSWeaponBase* pWeapon = EquipComponentInst->GetWeaponActor();
	if (!ensure(pWeapon)) return;

	FireComponentInst->StartReload(pWeapon);
}

void ATPSSoldierBase::OnReloadStateChanged(bool bIsReloading)
{
	if (!ensure(StateComponentInst.Get())) return;

	if (bIsReloading)
	{
		StateComponentInst->AddState(EActionState::Reloading);
	}
	else
	{
		StateComponentInst->RemoveState(EActionState::Reloading);
	}
}

void ATPSSoldierBase::OnFireStateChanged(bool bIsFiring)
{
	if (!ensure(StateComponentInst.Get())) return;

	if (bIsFiring)
	{
		StateComponentInst->AddState(EActionState::Firing);
	}
	else
	{
		StateComponentInst->RemoveState(EActionState::Firing);
	}
}